#include <cmath>
#include "FmOperator.h"
#include "Instrument.h"
#include "Program.h"

// Eight velocity sensitivity curves with 0 - 60 dB dynamic range
// velocity: 0-127 (a la MIDI)
// velocity sensitivity: 0-7 ("curve")
static float velocityScaling(unsigned velocity, unsigned vels) {
  static float logRange[8]={
      0.00f, // 0 dB dynamic range
     -1.00f, // 6 dB
     -2.00f, // 12 dB
     -3.32f, // 20 dB
     -5.00f, // 30 dB
     -6.65f, // 40 dB
     -8.32f, // 50 dB
    -10.00f, // 60 dB
  };

  if (velocity>127) velocity=127;
  if (vels>7) vels=7;
  return exp2f((127-velocity)*logRange[vels]/128);
}

static inline int clamp(int x, int min, int max) {
  return (x<min)? min : (x>max)? max : x;
}

// Scale the envelope times by a factor of 2^(-rs/128) over the eight octaves
// between A0 and A8.
// For rs=0, there is no scaling
// For rs=7, there is a factor of 2^5.25 over 8 octaves
static float timeScaling(int key, int rs) {
  static constexpr int KEY_A0 = 21;
  static constexpr int KEY_A8 = 117;
  int x = clamp(key, KEY_A0, KEY_A8) - KEY_A0;
  return exp2f(-x*rs/128.0);
}

static float keyScaling(bool expCurve, int depth, int x) {
  static constexpr float expCoeff=0.1069336;
  
  if (depth==0 || x==0)
    return 1.0f;
  float l = (expCurve)? exp2(-(72-x)*expCoeff-8) : ldexpf(25*x,-13);
  return exp2(l*depth);
}

static float keyScaling(const KeyScalingParam *param, unsigned key) {
  return (key<param->bp)? keyScaling(param->lcExp, param->lDepth, param->bp-key)
    : keyScaling(param->rcExp, param->rDepth, key-param->bp);
}

void FmOperator::noteOn(const FmOperatorParam *param,
			unsigned key,
			unsigned vel,
			std::int32_t deltaPhi,
			float levelCom) {
  mParam=param;
  // oscillator mode (fixed frequency or ratio)
  mCurrDeltaPhi=mDeltaPhiKey=(param->fixedFreq)?
    theOp6Daisy.freqToPhaseIncrement(param->freq)
    : param->freq*deltaPhi;

  // delays
  mDelay1=mDelay2=0;
  
  // envelope
  float level=param->totalLevel*keyScaling(&param->keyScaling, key);
  // +LIN and +EXP curves can only use the headroom above totalLevel
  // that is: keyScaling shouldn't cause level greater than unity.
  if (level > 1.0f) level=1.0f;
  level*=levelCom * velocityScaling(vel, param->velScaling);
  float timeS=timeScaling(key, param->kbdRateScaling);
  mEnvelope.noteOn(&param->envelope, level, timeS);
  mCurrAm=1.0;
}

void FmOperator::noteOff(const FmOperatorParam *param) {
  mEnvelope.noteOff(&param->envelope);
}

static const float sensitivity[]={ 0, 0.25, 0.5, 1.0 };

void FmOperator::fillBuffer(float *out,
			    const float *in,
			    const float *mod,
			    float pitchMod,
			    float ampMod,
			    int feedback) {
  unsigned phi=mPhi;
  // Pitch modulation
  int nextDeltaPhi=mDeltaPhiKey*pitchMod;
  int d2Phi=(nextDeltaPhi-mCurrDeltaPhi)/BLOCK_SIZE;
  
  int delay1=mDelay1;
  int delay2=mDelay2;
  // Amplitude modulation (in addition to envelope)
  float linAm=mCurrAm;
  float nextAm=exp2f(-ampMod*sensitivity[mParam->ams]);
  float dA=(nextAm-mCurrAm)/BLOCK_SIZE;
  
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    // Modulator is scaled by 4PI, which is the modulation index for ouput 99
    // Feedback is scaled similarly, but there is also an implicit
    // averaging of y1 and y2 and a scaling by 2^(feedback-9):
    // (2^k)PI, where k=2 {for 4PI} -1 {1/2 averaging} + feedback-9 =
    // = feedback-8.
    // To make things even more convolved, the averaged delays are in Q23
    // format (thus already shifted >> 8), which means that <<feedback
    // is all that remains.
    // The regular modulator, mod[i], is converted into Q31 fixed point
    // representation and multiplied by four (exp=30 to avoid overflow, so
    // we have to <<3 instead of <<2 to achieve *4).
    int mint=(feedback)? (delay1+delay2)<<feedback
                       : ((int) ldexpf(mod[i],30))<<3;
    float s=sinf((phi+mint)*ldexpf(M_PI,-31));
    float gain=mEnvelope.ProcessSample()*linAm;
    float y=s*gain;

    delay2=delay1;
    delay1=ldexpf(y,23);
    phi+=mCurrDeltaPhi;
    mCurrDeltaPhi+=d2Phi;
    linAm+=dA;
    out[i]=in[i] + y;
  }

  // Write back updated state
  mPhi=phi;
  mDelay1=delay1;
  mDelay2=delay2;
  mEnvelope.updateAfterBlock(&mParam->envelope);
  mCurrAm=linAm;
}

void FmOperator::fillBufferFb2(FmOperator op[],
			       float *out,
			       const float *in,
			       float pitchMod,
			       float ampMod,
			       int feedback) {
  FmOperator *op0=&op[0];
  FmOperator *op1=&op[1];
  unsigned phi0=op0->mPhi;
  unsigned phi1=op1->mPhi;

  // Pitch modulation
  int nextDeltaPhi=op0->mDeltaPhiKey*pitchMod;
  int currDeltaPhi0=op0->mCurrDeltaPhi;
  int d2Phi0=(nextDeltaPhi-currDeltaPhi0)/BLOCK_SIZE;
  nextDeltaPhi=op1->mDeltaPhiKey*pitchMod;
  int currDeltaPhi1=op1->mCurrDeltaPhi;
  int d2Phi1=(nextDeltaPhi-currDeltaPhi1)/BLOCK_SIZE;

  // Amplitude modulation (in addition to envelope)
  float nextAm=exp2f(-ampMod*sensitivity[op0->mParam->ams]);
  float linAm0=op0->mCurrAm;
  float dA0=(nextAm-linAm0)/BLOCK_SIZE;
  nextAm=exp2f(-ampMod*sensitivity[op1->mParam->ams]);
  float linAm1=op1->mCurrAm;
  float dA1=(nextAm-linAm1)/BLOCK_SIZE;

  // Feedback
  int delay1=op0->mDelay1;
  int delay2=op0->mDelay2;
  
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    // First operator, op0
    int mint=(feedback)? (delay1+delay2)<<feedback : 0;
    float s=sinf((phi0+mint)*ldexpf(M_PI,-31));
    float gain=op0->mEnvelope.ProcessSample()*linAm0;
    float y=s*gain;

    phi0+=currDeltaPhi0;
    currDeltaPhi0+=d2Phi0;
    linAm0+=dA0;

    // second operator, op1
    mint=((int) ldexpf(y,30))<<3;
    s=sinf((phi1+mint)*ldexpf(M_PI,-31));
    gain=op1->mEnvelope.ProcessSample()*linAm1;
    y=s*gain;

    delay2=delay1;
    delay1=ldexpf(y,23);
    phi1+=currDeltaPhi1;
    currDeltaPhi1+=d2Phi1;
    linAm1+=dA1;
    out[i]=in[i] + y;
  }

  // Write back updated state
  op0->mDelay1=delay1;
  op0->mDelay2=delay2;
  op0->mPhi=phi0;
  op0->mCurrDeltaPhi=currDeltaPhi0;
  op0->mCurrAm=linAm0;
  op1->mPhi=phi1;
  op1->mCurrDeltaPhi=currDeltaPhi1;
  op1->mCurrAm=linAm1;
  // update envelope state
  op0->mEnvelope.updateAfterBlock(&op0->mParam->envelope);
  op1->mEnvelope.updateAfterBlock(&op1->mParam->envelope);
}

void FmOperator::fillBufferFb3(FmOperator op[],
			       float *out,
			       const float *in,
			       float pitchMod,
			       float ampMod,
			       int feedback) {
  FmOperator *op0=&op[0];
  FmOperator *op1=&op[1];
  FmOperator *op2=&op[2];
  unsigned phi0=op0->mPhi;
  unsigned phi1=op1->mPhi;
  unsigned phi2=op2->mPhi;

  // Pitch modulation
  int nextDeltaPhi=op0->mDeltaPhiKey*pitchMod;
  int currDeltaPhi0=op0->mCurrDeltaPhi;
  int d2Phi0=(nextDeltaPhi-currDeltaPhi0)/BLOCK_SIZE;
  nextDeltaPhi=op1->mDeltaPhiKey*pitchMod;
  int currDeltaPhi1=op1->mCurrDeltaPhi;
  int d2Phi1=(nextDeltaPhi-currDeltaPhi1)/BLOCK_SIZE;
  nextDeltaPhi=op2->mDeltaPhiKey*pitchMod;
  int currDeltaPhi2=op2->mCurrDeltaPhi;
  int d2Phi2=(nextDeltaPhi-currDeltaPhi2)/BLOCK_SIZE;

  // Amplitude modulation (in addition to envelope)
  float nextAm=exp2f(-ampMod*sensitivity[op0->mParam->ams]);
  float linAm0=op0->mCurrAm;
  float dA0=(nextAm-linAm0)/BLOCK_SIZE;
  nextAm=exp2f(-ampMod*sensitivity[op1->mParam->ams]);
  float linAm1=op1->mCurrAm;
  float dA1=(nextAm-linAm1)/BLOCK_SIZE;
  nextAm=exp2f(-ampMod*sensitivity[op2->mParam->ams]);
  float linAm2=op2->mCurrAm;
  float dA2=(nextAm-linAm2)/BLOCK_SIZE;
  
  // Feedback
  int delay1=op0->mDelay1;
  int delay2=op0->mDelay2;
  
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    // First operator, op0
    int mint=(feedback)? (delay1+delay2)<<feedback : 0;
    float s=sinf((phi0+mint)*ldexpf(M_PI,-31));
    float gain=op0->mEnvelope.ProcessSample()*linAm0;
    float y=s*gain;

    phi0+=currDeltaPhi0;
    currDeltaPhi0+=d2Phi0;
    linAm0+=dA0;

    // second operator, op1
    mint=((int) ldexpf(y,30))<<3;
    s=sinf((phi1+mint)*ldexpf(M_PI,-31));
    gain=op1->mEnvelope.ProcessSample()*linAm1;
    y=s*gain;
    
    phi1+=currDeltaPhi1;
    currDeltaPhi1+=d2Phi1;
    linAm1+=dA1;

    // third operator, op2
    mint=((int) ldexpf(y,30))<<3;
    s=sinf((phi2+mint)*ldexpf(M_PI,-31));
    gain=op2->mEnvelope.ProcessSample()*linAm2;
    y=s*gain;
    
    delay2=delay1;
    delay1=ldexpf(y,23);
    phi2+=currDeltaPhi2;
    currDeltaPhi2+=d2Phi2;
    linAm2+=dA2;
    out[i]=in[i] + y;
  }

  // Write back updated state
  op0->mDelay1=delay1;
  op0->mDelay2=delay2;
  op0->mPhi=phi0;
  op0->mCurrDeltaPhi=currDeltaPhi0;
  op0->mCurrAm=linAm0;
  op1->mPhi=phi1;
  op1->mCurrDeltaPhi=currDeltaPhi1;
  op1->mCurrAm=linAm1;
  op2->mPhi=phi2;
  op2->mCurrDeltaPhi=currDeltaPhi2;
  op2->mCurrAm=linAm2;

  // update envelope state
  op0->mEnvelope.updateAfterBlock(&op0->mParam->envelope);
  op1->mEnvelope.updateAfterBlock(&op1->mParam->envelope);
  op2->mEnvelope.updateAfterBlock(&op2->mParam->envelope);
}
