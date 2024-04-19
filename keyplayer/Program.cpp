#include <cmath>
#include "configuration.h"
#include "Program.h"
#include "SyxBulkFormat.h"

// This is lfo speed 35 (SYX parameter) 5.794 Hz
static constexpr int initLfoSpeed=ldexpf(5.794f*BLOCK_SIZE/SAMPLE_RATE, 32);

LfoParam::LfoParam()
  : waveform{0}, delay{0}, deltaPhi{initLfoSpeed}
{
}

EnvelopeParam::EnvelopeParam(float l0)
  : level0{l0},
    levels{1.0f, 1.0f, 1.0f, l0},
    times{0.001500, 0.001500, 0.001500, 0.001500}
{
}

KeyScalingParam::KeyScalingParam()
  : bp{60}, lcExp{false}, lDepth{0}, rcExp{false}, rDepth{0}
{
}

FmOperatorParam::FmOperatorParam()
  : fixedFreq{false},
    velScaling{0},
    kbdRateScaling{0},
    ams{0},
    freq{1.0f},
    totalLevel{0.0f},
    envelope{0.0f}
{
}

Program::Program() 
  : algorithm{1},
    feedback{0},
    lfoPmDepth{0.0f},
    lfoAmDepth{0.0f},
    pitchEnvelope{1.0f}
{
  op[5].totalLevel=1.0f;
}

static const Program initVoice;
static Program programBank[32];

const Program *Program::getProgram(unsigned programNumber) {
  constexpr unsigned N=sizeof(programBank)/sizeof(Program);
  return (programNumber==0 || programNumber>N)?
    &initVoice : &programBank[programNumber-1];
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

static inline unsigned clamp(unsigned x, unsigned max) {
  return x<=max? x : max; 
}

// Level, negative logcale 2^(-t/8), so unit is 1/8 octave
static int logLevel(int tl99) {
  static const int first20[]={
    127, 122, 118, 114, 110, 107, 104, 102, 100,  98,
     96,  94,  92,  90,  88,  86,  85,  84,  82,  81
  };

  return (tl99<20)? first20[tl99] : 99-tl99;
}

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

static float computeAmpLevel(unsigned l99) {
  return (l99==0)? 0.0f : exp2f(-logLevel(l99)/8.0);
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

static float computePitchLevel(unsigned l99) {
  static const signed char first[18]={
    0, 12, 24, 33, 43, 52, 60, 67, 72, 76, 79, 82, 85, 87, 89, 91, 93, 95
  };
  int t=(l99<18)? first[l99] :
        (l99<82)? l99+78 :
        (l99<99)? 256-first[99-l99] : 256;

  return exp2f((t-128)/128.0);
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
  unsigned bp=60;
  bool lcExp=false;
  int lDepth=0;
  bool rcExp=false;
  int rDepth=0;
  
  // Breakpoint
  if (syxKls.ld || syxKls.rd) {
    // bp=39 ~ C5 (i.e. midi key 60),
    // bp=0  ~ A1 (21), bp=99 ~ C10 (120)
    bp = clamp(syxKls.bp, 99) + 21;  
  }

  // Left curve
  if (syxKls.ld) {
    // curves: -LIN (0) -EXP (1) +EXP(2) +LIN(3)
    unsigned curve=syxKls.rc_lc & 3;
    int depth=clamp(syxKls.ld, 99);
    lcExp=(curve==1) || (curve==2);
    lDepth=(curve<2)? -depth : depth;
  }

  // Right curve
  if (syxKls.rd) {
    // curves: -LIN (0) -EXP (1) +EXP(2) +LIN(3)
    unsigned curve=(syxKls.rc_lc >> 2) & 3;
    int depth=clamp(syxKls.rd, 99);
    rcExp=(curve==1) || (curve==2);
    rDepth=(curve<2)? -depth : depth;
  }
  
  kls.bp=bp;
  kls.lcExp=lcExp;
  kls.lDepth=lDepth;
  kls.rcExp=rcExp;
  kls.rDepth=rDepth;
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

static void convert(const SyxVoiceParam::Op &syxOp, FmOperatorParam &op) {
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

static float computeLfoPmDepth(unsigned pmd, unsigned lpms) {
  static const float sensitivity[8]={      0, 0.039f, 0.078f, 0.129f,
       	       	                      0.215f, 0.333f, 0.600f, 0.999 };
  return sensitivity[lpms]*pmd/99.0f;
}

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

static void convert(const SyxVoiceParam::Lfo &syxLfo, LfoParam lfo) {
  // LFO waveform 0-5: TRIANGL, SAW DWN, SAW UP, SQUARE, SINE, S/HOLD
  lfo.waveform=clamp((syxLfo.lpms_wave_sync >> 1) & 7, 5);
  lfo.delay=computeDelayInBlocks(clamp(syxLfo.delay, 99));
  lfo.deltaPhi=computeLfoDeltaPhi(clamp(syxLfo.speed, 99));
}

void Program::load(const SyxVoiceParam &syxVoice) {
  unsigned fbl, lpms;
  
  algorithm=clamp(syxVoice.algorithm, 31)+1;
  
  // Map fbl 0..7 onto 0, 3..9
  fbl=syxVoice.opi_fbl & 7;
  feedback=(fbl)? fbl+2 : 0;
  
  // Map lpms and pmd to lfoPmDepth (float)
  lpms=clamp(syxVoice.lfo.lpms_wave_sync>>4, 7);
  lfoPmDepth=computeLfoPmDepth(clamp(syxVoice.lfo.pmd, 99), lpms);
  // Map amd to lfoAmDepth (float)
  lfoAmDepth=computeLfoAmDepth(clamp(syxVoice.lfo.amd, 99));

  convert(syxVoice.lfo, lfo);
  convertPitchEnvelope(syxVoice.pitchEnvelope, pitchEnvelope);
  for (unsigned i=0; i<6; ++i)
    convert(syxVoice.op[i], op[i]);
}

#include "rom1a.i"

void Program::load_rom1a() {
  const SyxBulkFormat *syxFile=reinterpret_cast<const SyxBulkFormat*>(rom1a);

  for (unsigned i=0; i<32; ++i)
    programBank[i].load(syxFile->voiceParam[i]);
}
