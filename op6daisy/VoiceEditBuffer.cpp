#include <cmath>
#include "VoiceEditBuffer.h"
#include "SyxBulkFormat.h"


VoiceEditBuffer::VoiceEditBuffer()
  : mOpEnabled{(1<<NUM_OPERATORS)-1} // All operators on
{
}

void VoiceEditBuffer::loadInitialProgram() {
  Program initialProgram;
  loadProgram(initialProgram);
}

void VoiceEditBuffer::loadProgram(const Program &program) {
  mProgram=program;
  mOpEnabled=(1<<NUM_OPERATORS)-1; // All operators on
}

void VoiceEditBuffer::storeProgram(Program &program) const {
  program=mProgram;
  // Turn off output if operator is disabled
  for (unsigned i=0, b=1; i<NUM_OPERATORS; ++i, b<<=1)
    if ((mOpEnabled & b)==0)
      program.op[i].totalLevel=0;
}

void VoiceEditBuffer::setOperatorEnable(unsigned page, bool isEnabled) {
  // page: the operators are numbered Op6(0), Op5(1), ..., Op1(5)
  if (isEnabled)
    mOpEnabled |= 1<<page;
  else
    mOpEnabled &= ~(1<<page);
}

static void convertOp(const SyxVoiceParam::Op &syxOp, FmOperatorParam &op);
static void convertCommon(const SyxVoiceParam &syxVoice,
			  Program &common);

void VoiceEditBuffer::loadSyx(const SyxVoiceParam &syxVoice) {
  // Start from initial program
  loadInitialProgram();
  // Convert the operator parameters
  for (unsigned i=0; i<6; ++i)
    convertOp(syxVoice.op[i], mProgram.op[i]);
  // Convert the common voice parameters
  convertCommon(syxVoice, mProgram);
}


//
// Voice parameter settings
//

enum EnvelopeParameters {
  kEnvelopeTime1,
  kEnvelopeTime2,
  kEnvelopeTime3,
  kEnvelopeTime4,
  kEnvelopeLevel0,
  kEnvelopeLevel1,
  kEnvelopeLevel2,
  kEnvelopeLevel3,
  kEnvelopeLevel4,
  NUM_ENVELOPE_PARAMETERS
};

enum OpParameters {
  kKlsBreakpoint=NUM_ENVELOPE_PARAMETERS,
  kKlsLeftDepth,
  kKlsLeftCurve,
  kKlsRightDepth,
  kKlsRightCurve,
  kFixedFrequency,
  kFrequencyRatio,
  kTotalOutputLevel,
  kAmSensitivity,
  kVelocitySensitivity,
  kKeyboardToEnvelopeRate,
  kOperatorEnable,
  NUM_OP_PARAMETERS
};

enum ComonParameters {
  kAlgorithm=NUM_ENVELOPE_PARAMETERS,
  kFeedback,
  kOscillatorSync,
  kPitchEnvelopeDepth,
  kPmSensitivity,
  kVelocityToPitchEnvelopeDepth,
  kKeyboardToPitchEnvelopeRate,
  kLfoSpeed,
  kLfoDelay,
  kLfoWaveform,
  kWasLfoSyncNotUsed,
  kLfoPmDepth,
  kLfoAmDepth,
  NUM_COMMON_PARAMETERS
};

static void setOpParameter(FmOperatorParam &op, unsigned param, unsigned x);
static void setCommonParameter(Program &common, unsigned param, unsigned x);

void VoiceEditBuffer::setParameter(unsigned page,
				   unsigned paramNumber,
				   unsigned paramValue) {
  if (page<6)
    if (paramNumber==kOperatorEnable)
      setOperatorEnable(page, paramValue!=0);
    else
      setOpParameter(mProgram.op[page], paramNumber, paramValue);
  else if (page==6)
    setCommonParameter(mProgram, paramNumber, paramValue);
}

static inline unsigned paramMsb(unsigned x) {
  // general 14-bit parameter to high 7 bits
  return (x >> 7);
}

static inline unsigned clamp(unsigned x, unsigned max) {
  return x<=max? x : max; 
}

static float paramToEnvelopeTime(unsigned t99) {
  if (t99>=12) {
    // Time doubles every 8 snaps
    return 20.032f*exp2f(((int)t99-99)/8.0);
  }
  unsigned blocks=(t99==11)? 14 : t99+2;
  return blocks*(0.002f/3);
}

// Level, negative logcale 2^(-t/8), so unit is 1/8 octave
static int logLevel(int tl99) {
  static const int first20[]={
    127, 122, 118, 114, 110, 107, 104, 102, 100,  98,
     96,  94,  92,  90,  88,  86,  85,  84,  82,  81
  };

  return (tl99<20)? first20[tl99] : 99-tl99;
}

static float computeAmpLevel(unsigned l99) {
  return (l99==0)? 0.0f : exp2f(-logLevel(l99)/8.0);
}

static void setAmEnvelopeParameter(EnvelopeParam &envelope,
				   unsigned paramNumber,
				   unsigned x) {
  unsigned msb=clamp(paramMsb(x), 99);
  if (paramNumber<=kEnvelopeTime4)
    envelope.times[paramNumber-kEnvelopeTime1]=paramToEnvelopeTime(msb);
  else if (paramNumber<=kEnvelopeLevel4) {
    float level=computeAmpLevel(msb);
    if (paramNumber==kEnvelopeLevel0)
      envelope.level0=level;
    else
      envelope.levels[paramNumber-kEnvelopeLevel1]=level;
  }
}

static void setKeyScalingParameter(KeyScalingParam &ks,
				   unsigned paramNumber,
				   unsigned x) {
  x=paramMsb(x);
  switch (paramNumber) {
  case kKlsBreakpoint: ks.bp=x; break;
  case kKlsLeftDepth:  ks.left.depth=clamp(x, 99); break;
  case kKlsLeftCurve: ks.left.curve=x & 3;
  case kKlsRightDepth: ks.right.depth=clamp(x, 99); break;
  case kKlsRightCurve: ks.right.curve=x & 3;
  }
}

// Frequency is transferred as a 14-bit fixed point number, x:
// eee|mmmmmmmmmmmm with an implicit one in the mantissa: 1.mmmmmmmmmmmm
// except when e=0.
// e=0 range [0,1) ulp=2^(-11)  e=4 range [8,16) ulp=2^(-8)
// e=1       [1,2)     2^(-11)  e=5      [16,32)     2^(-7)
// e=2       [2,4)     2^(-10)  e=6      [32,64)     2^(-6)
// e=3       [4,8)     2^(-9)   e=7      [64,128)    2^(-5)
//
// Optionally, "noise" is added when converting float

static float unpackFp14(unsigned x14, bool addNoise=false) {
  unsigned e=x14>>11;
  unsigned m=x14 & 0x7ff;

  if (e!=0) {
    m|=0x800; // implicit leading one
    e--;
  }
  unsigned i=m & 0xf;
  m<<=12; // now the mantissa is 24-bit
  if (addNoise) {
    static const short noise[16]= {
         0,  465,  714, -491,  132,  773, -133,  137,
      -235, -280,  -33,-1452, -245,  726, -189, -586
    };
    m+=noise[i];
  }

  return ldexpf(m, e-23);
}

static void setOpParameter(FmOperatorParam &op, unsigned param, unsigned x) {
  if (param<NUM_ENVELOPE_PARAMETERS)
    setAmEnvelopeParameter(op.envelope, param, x);
  else if (param<=kKlsRightCurve)
    setKeyScalingParameter(op.keyScaling, param, x);
  else {
    unsigned msb=clamp(paramMsb(x), 99);
    switch (param) {
    case kFixedFrequency:
      // Fixed frequency range [0, 16383.9)
      op.fixedFreq=true;
      op.freq=128*unpackFp14(x);
      break;
    case kFrequencyRatio:
      // Frequency ratio range [0, 63.9)
      op.fixedFreq=false;
      op.freq=unpackFp14(x, true);
      break;
    case kTotalOutputLevel:
      op.totalLevel=computeAmpLevel(msb);
      break;
    case kAmSensitivity:
      op.ams=clamp(msb,3);
      break;
    case kVelocitySensitivity:
      op.velScaling=clamp(msb,7);
      break;
    case kKeyboardToEnvelopeRate:
      op.kbdRateScaling=clamp(msb,7);
      break;
    }
  }
}

// The unsigned scale 0..99, where 50 is unity (as used by DX7)
static float computePitchLevel(unsigned l99) {
  static const signed char first[18]={
    0, 12, 24, 33, 43, 52, 60, 67, 72, 76, 79, 82, 85, 87, 89, 91, 93, 95
  };
  int t=(l99<18)? first[l99] :
        (l99<82)? l99+78 :
        (l99<99)? 256-first[99-l99] : 256;

  return exp2f((t-128)/128.0);
}

// Level +/-99 with unity at zero, as used by op6
static float paramToPmEnvelopeLevel(signed char s8) {
  unsigned l99 = (s8>>1) + 50;
  return computePitchLevel(l99);
}

static void setPmEnvelopeParameter(EnvelopeParam &envelope,
				   unsigned paramNumber,
				   unsigned x) {
  if (paramNumber<=kEnvelopeTime4) {
    unsigned msb=clamp(paramMsb(x), 99);
    envelope.times[paramNumber-kEnvelopeTime1]=paramToEnvelopeTime(msb);
  }
  else if (paramNumber<=kEnvelopeLevel4) {
    // convert 14-bit x to signed format +/-127
    signed char level_s8=x>>6; 
    float level=paramToPmEnvelopeLevel(level_s8);
    if (paramNumber==kEnvelopeLevel0)
      envelope.level0=level;
    else
      envelope.levels[paramNumber-kEnvelopeLevel1]=level;
  }
}

// LFO pitch-modulation sensitivity (DX has 0..7 range)
static const float lfoPmSensitivity[8]={      0, 0.039f, 0.078f, 0.129f,
					 0.215f, 0.333f, 0.600f, 0.999 };

// LFO depth in negative log representation: 2^(-t/256)
// (1-LFO) is scaled by t/256 before expo-step. In this way we get a linear
// scale factor between 1.0 (0 dB) and 2^(-2.828*2) (-34 dB) at amd=99.
static float computeLfoAmDepth(unsigned amd) {
  static const short last20[20]={
    0x2d4, 0x1cc, 0x153, 0x110, 0x0e3, 0x0c1, 0x0a7, 0x092, 0x080, 0x076,
    0x06f, 0x06a, 0x066, 0x066, 0x062, 0x05b, 0x058, 0x055, 0x053, 0x051
  };
  short t=(amd>=80)? last20[99-amd] : amd;
  return t/256.0;
}

static int computeLfoDeltaPhi(unsigned lfoSpeed) {
  unsigned factor, phaseInc;

  // Patch activate/scale value [0,99] -> [11,8670]
  lfoSpeed=(lfoSpeed)? (660*lfoSpeed)>>8 : 1;
  factor=(lfoSpeed>160)? ((lfoSpeed-160)>>2) + 11 : 11;
  phaseInc=lfoSpeed*factor;

  // Scale to 32-bit counters etc
  // A factor of 2^16 is due to 32 vs 16-bit phase accumulation
  // Our block size and sample rate vs those of DX7
  // (guessing 128 samples at 49096Hz).
  return (phaseInc<<16)*(49096.0f/128 * BLOCK_SIZE/SAMPLE_RATE);
}

static unsigned short computeDelayInBlocks(unsigned syxDelay) {
  unsigned t1, t2, lfoDelayIncrement, nBlocks;

  // Compute lfo delay increment 0..99 -> 64..4864  (99 -> 64)
  t1=99-syxDelay;
  t2=(t1>>4)+2;
  lfoDelayIncrement=((t1 & 15) + 16) << t2;

  // Number of blocks until delay accumulator overflows (rounded upwards)
  nBlocks=(65536-1+lfoDelayIncrement)/lfoDelayIncrement;

  // Scale to our block size and sample rate
  // vs those of DX7 (guessing 128 samples at 49096Hz).
  return nBlocks*((float) SAMPLE_RATE/BLOCK_SIZE * 128/49096);
}

static void setCommonParameter(Program &common, unsigned param, unsigned x) {
  if (param<NUM_ENVELOPE_PARAMETERS)
    setPmEnvelopeParameter(common.pitchEnvelope, param, x);
  else {
    unsigned msb=clamp(paramMsb(x), 99);
    
    switch (param) {
    case kAlgorithm:
      common.algorithm=clamp(msb, 31);
      break;
    case kFeedback:
      common.feedback=clamp(msb, 9); // NB: DX [0,7] is mapped onto [0,9]
      break;
      //  TODO: kOscillatorSync,
      //  TODO: (remove) kPitchEnvelopeDepth,
      //  TODO: kVelocityToPitchEnvelopeDepth,
      //  TODO: kKeyboardToPitchEnvelopeRate,
    case kLfoSpeed:
      common.lfo.deltaPhi=computeLfoDeltaPhi(msb);
      break;
    case kLfoDelay:
      common.lfo.delay=computeDelayInBlocks(msb);
      break;
    case kLfoWaveform:
      // LFO waveform 0-5: TRIANGL, SAW DWN, SAW UP, SQUARE, SINE, S/HOLD
      common.lfo.waveform=clamp(msb, 5);
      break;
    case kLfoAmDepth:
      common.lfoAmDepth=computeLfoAmDepth(msb);
      break;
    case kPmSensitivity:
      common.lfoPmSensitivity=lfoPmSensitivity[clamp(msb, 8)];
      break;
    case kLfoPmDepth:
      common.lfoPmInitDepth=clamp(msb, 99);
      break;
    }
  }
}

//
// Conversion SyxVoiceParam -> Program
//

// Ensure that the Syx structs have the expected sizes
static_assert(sizeof(Syx::Preamble)==6);
static_assert(sizeof(SyxVoiceParam::Envelope)==8);
static_assert(sizeof(SyxVoiceParam::KbdLevelScaling)==4);
static_assert(sizeof(SyxVoiceParam::Op)==17);
static_assert(sizeof(SyxVoiceParam::Lfo)==5);
static_assert(sizeof(SyxVoiceParam)==128);
static_assert(sizeof(Syx::Postamble)==2);
static_assert(sizeof(SyxBulkFormat)==SyxBulkFormat::Size);

static float level_dB(int tl99) {
  int x=logLevel(tl99);
  return -x*(20*log10f(2)/8);
}

// rate in dB/s
static float rate_dBps(int rate99) {
  int x=41*rate99/16;

  return 0.2819f*exp2f(x/16.0);
}

static constexpr float MIN_ENVELOPE_TIME=0.004f/3;
static constexpr float MAX_ENVELOPE_TIME=20.032f;

// l1, l2 levels (unit: 1/8 octave)
// r99 is the "r99" rate
static float matchEnvelopeTime(int r99, int l1, int l2) {
  float dB1=level_dB(l1);
  float dB2=level_dB(l2);
  float dBps=rate_dBps(r99);
  float t=fabsf(dB1-dB2)/dBps;
  
  if (t<MIN_ENVELOPE_TIME) t=MIN_ENVELOPE_TIME;
  if (t>MAX_ENVELOPE_TIME) t=MAX_ENVELOPE_TIME;
  return t;
}

// Matches decay rate by solving for t.
static float matchEnvelopeRate(int r99) {
  float dBps=rate_dBps(r99);
  // "our" definition of envelope time is decay to exp(-pi), 0.0432 (-27 dB)
  float t=27.287527/dBps;

  if (t<MIN_ENVELOPE_TIME) t=MIN_ENVELOPE_TIME;
  if (t>MAX_ENVELOPE_TIME) t=MAX_ENVELOPE_TIME;
  return t;
}

static void convertEnvelopeTimes(const SyxVoiceParam::Envelope &syxEnv,
				 EnvelopeParam &env)
{
  unsigned char l0=clamp(syxEnv.level[3], 99); // l0 and l4 the same
  unsigned char l1=clamp(syxEnv.level[0], 99);
  unsigned char l2=clamp(syxEnv.level[1], 99);
  unsigned char rate;
  // No perfect way of doing this...
  // Let the rate and the difference between levels determine first two segments
  // In this way the envelope time is matched, makes sense for attack.
  rate=clamp(syxEnv.rate[0], 99);
  env.times[0]=matchEnvelopeTime(rate, l0, l1);
  rate=clamp(syxEnv.rate[1], 99);
  env.times[1]=matchEnvelopeTime(rate, l1, l2);
  // Convert the rate for the last two segments
  // Makes sense for decaying segments
  rate=clamp(syxEnv.rate[2], 99);
  env.times[2]=matchEnvelopeRate(rate);
  rate=clamp(syxEnv.rate[3], 99);
  env.times[3]=matchEnvelopeRate(rate);
}

static void convertAmpEnvelope(const SyxVoiceParam::Envelope &syxEnv,
			       EnvelopeParam &env)
{
  for (unsigned i=0; i<4; ++i) {
    env.levels[i]=computeAmpLevel(clamp(syxEnv.level[i], 99));
  }
  env.level0=env.levels[3];

  convertEnvelopeTimes(syxEnv, env);
}

static void convertPitchEnvelope(const SyxVoiceParam::Envelope &syxEnv,
				 EnvelopeParam &env)
{
  for (unsigned i=0; i<4; ++i) {
    env.levels[i]=computePitchLevel(clamp(syxEnv.level[i], 99));
  }
  env.level0=env.levels[3];

  // FIXME: envelope times are set for AM envelope.
  convertEnvelopeTimes(syxEnv, env);
}

static void convert(const SyxVoiceParam::KbdLevelScaling &syxKls,
		    KeyScalingParam &kls)
{
  // Breakpoint
  if (syxKls.ld || syxKls.rd) {
    // bp=39 ~ C5 (i.e. midi key 60),
    // bp=0  ~ A1 (21), bp=99 ~ C10 (120)
    kls.bp = clamp(syxKls.bp, 99) + 21;
  }

  // Left curve
  if (syxKls.ld) {
    // curves: -LIN (0) -EXP (1) +EXP(2) +LIN(3)
    kls.left.curve=syxKls.rc_lc & 3;
    kls.left.depth=clamp(syxKls.ld, 99);
  }

  // Right curve
  if (syxKls.rd) {
    // curves: -LIN (0) -EXP (1) +EXP(2) +LIN(3)
    kls.right.curve=(syxKls.rc_lc >> 2) & 3;
    kls.right.depth=clamp(syxKls.rd, 99);
  }
}

static float computeFrequency(bool fixed,
			      unsigned coarse,
			      unsigned fine,
			      unsigned detune)
{
  static const signed char cents[15]={-41, -35, -29, -23, -17, -11, -5,
				        0,   5,  11,  17,  23,  29, 35, 41};
  float f;
  if (fixed) {
    // pm=1 (fixed) coarse selects 1, 10, 100, 1000 Hz
    // fine is a fractional, base-10 exponent
    constexpr float LOG2_10=3.321928095f;
    f=exp2f(((coarse % 4) + fine*0.01)*LOG2_10);
  }
  else {
    // pm=0 (ratio) coarse selects 0.5, 1, 2, 3,..., 31
    // fine is 1/100 of coarse
    f = (coarse==0)? 0.5 : coarse;
    f += fine*f*0.01;
  }
	
  // detune=7 is zero cents, <7 means negative, >7 positive
  return f*exp2f(cents[detune]/1200.0f); 
}

static void convertOp(const SyxVoiceParam::Op &syxOp, FmOperatorParam &op) {
  bool pm = syxOp.freqCoarse_pm & 1;
  unsigned freqCoarse=clamp(syxOp.freqCoarse_pm>>1, 31);
  unsigned freqFine=clamp(syxOp.freqFine, 99);
  unsigned detune=clamp(syxOp.pd_krs>>3, 14);

  // Frequency
  op.fixedFreq = pm;
  op.freq=computeFrequency(pm, freqCoarse, freqFine, detune);
  // level
  op.totalLevel=computeAmpLevel(clamp(syxOp.tl, 99));
  // velocity sensitivity
  op.velScaling=clamp(syxOp.ts_ams>>2, 7);
  // AM sensitivity
  op.ams=syxOp.ts_ams & 3;
  // rate scaling
  op.kbdRateScaling=syxOp.pd_krs & 7;
  
  convertAmpEnvelope(syxOp.envelope, op.envelope);
  convert(syxOp.kls, op.keyScaling);
}

static void convert(const SyxVoiceParam::Lfo &syxLfo, LfoParam lfo) {
  // LFO waveform 0-5: TRIANGL, SAW DWN, SAW UP, SQUARE, SINE, S/HOLD
  lfo.waveform=clamp((syxLfo.lpms_wave_sync >> 1) & 7, 5);
  lfo.delay=computeDelayInBlocks(clamp(syxLfo.delay, 99));
  lfo.deltaPhi=computeLfoDeltaPhi(clamp(syxLfo.speed, 99));
}

static void convertCommon(const SyxVoiceParam &syxVoice, Program &common) {
  common.algorithm=clamp(syxVoice.algorithm, 31);
  
  // Map fbl 0..7 onto 0, 3..9
  unsigned fbl=syxVoice.opi_fbl & 7;
  common.feedback=(fbl)? fbl+2 : 0;
  
  // Map lpms -> lfoPmSensitivity (specific scale)
  unsigned lpms=clamp(syxVoice.lfo.lpms_wave_sync>>4, 7);
  common.lfoPmSensitivity=lfoPmSensitivity[lpms];
  // pmd (lfoPmInitDepth) is just a linear scale 0..99
  unsigned pmd=clamp(syxVoice.lfo.pmd, 99);
  common.lfoPmInitDepth=pmd;
  // Map amd to lfoAmDepth (float)
  common.lfoAmDepth=computeLfoAmDepth(clamp(syxVoice.lfo.amd, 99));

  convert(syxVoice.lfo, common.lfo);
  convertPitchEnvelope(syxVoice.pitchEnvelope, common.pitchEnvelope);
}

