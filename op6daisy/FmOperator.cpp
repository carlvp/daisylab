#include <cmath>
#include "FmOperator.h"
#include "Instrument.h"
#include "Program.h"
#include "sine_lut.h"

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

float KeyScalingCurve::keyScaling(unsigned key) const {
  static constexpr float k1_3=1.0f/3;
  static constexpr float k1_12=1.0f/12;
  static constexpr float k1_24=1.0f/24;
  static constexpr float depth_scale=255.0f/99/256; // Maps depth onto [0, 1.0)

  if (depth==0 || key==0)
    return 1.0f;
  int signedDepth=(curve==kMinusLin || curve==kMinusExp)? -depth : depth;

  // LIN: linear arg: key/3
  // EXP: linear for key=0,...,8 exponential from key=9
  float arg=(curve==kMinusExp || curve==kPlusExp)?
    ((key<=8)? key*k1_24 : exp2f(key*k1_12-2)) : key*k1_3;
  return exp2f(arg*signedDepth*depth_scale);
}


void FmOperator::changeKey(unsigned key, std::int32_t deltaPhi) {
  if (!mParam->fixedFreq) {
    mDeltaPhiKey=mParam->freq*deltaPhi;
    // TODO: keyscaling should be considered here, how?
  }
}

void FmOperator::noteOn(const FmOperatorParam *param,
			unsigned key,
			unsigned vel,
			std::int32_t deltaPhi,
			float levelCom,
			bool retrig) {
  mParam=param;
  // oscillator mode (fixed frequency or ratio)
  mCurrDeltaPhi=mDeltaPhiKey=(param->fixedFreq)?
    theOp6Daisy.freqToPhaseIncrement(param->freq)
    : param->freq*deltaPhi;

  // delays
  mDelay1=mDelay2=0;
  
  // envelope
  float level=param->totalLevel*param->keyScaling.keyScaling(key);
  // +LIN and +EXP curves can only use the headroom above totalLevel
  // that is: keyScaling shouldn't cause level greater than unity.
  if (level > 1.0f) level=1.0f;
  level*=levelCom * velocityScaling(vel, param->velScaling);
  float timeS=timeScaling(key, param->kbdRateScaling);
  mEnvelope.noteOn(&param->envelope, level, timeS, retrig);
  mCurrAm=1.0;
}

void FmOperator::noteOff(const FmOperatorParam *param) {
  mEnvelope.noteOff(&param->envelope);
}

static const float sensitivity[]={ 0, 0.25, 0.5, 1.0 };

// float constants for scaling of results
static constexpr float two_to_30=1073741824;
static constexpr float two_to_7=128;
static constexpr float two_to_m23=1.0f/8388608;

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
    // this range, modulo 2PI, is represented as a 32-bit integer (mod 2^32).
    // the factor 4PI thus corresponds to 2^33.
    //    
    // Feedback is scaled similarly, but there is also an implicit
    // averaging of delay1 and delay2 and a scaling by 2^(feedback-9).
    // The delays are represented as 24-bit fixed-points (Q23), which
    // means that the scaling boils down to (delay1+delay2)<<feedback.
    // The result (mod 2^32) reprents the real range [-PI,+PI).
    int mint=(feedback)? (delay1+delay2)<<feedback
                       : ((int) (mod[i]*two_to_30))<<3;
    int sint=sine_lut(phi+mint);
    float gain=mEnvelope.ProcessSample()*linAm;
    float y=sint*gain; // y is now scaled by a factor of 2^23

    delay2=delay1;
    delay1=(int) y;
    phi+=mCurrDeltaPhi;
    mCurrDeltaPhi+=d2Phi;
    linAm+=dA;
    out[i]=in[i] + y*two_to_m23;
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
    int sint=sine_lut(phi0+mint);
    float gain=op0->mEnvelope.ProcessSample()*linAm0;
    float y=sint*gain; // y is now scaled by a factor of 2^23

    phi0+=currDeltaPhi0;
    currDeltaPhi0+=d2Phi0;
    linAm0+=dA0;

    // second operator, op1
    mint=((int) (y*two_to_7))<<3;
    sint=sine_lut(phi1+mint);
    gain=op1->mEnvelope.ProcessSample()*linAm1;
    y=sint*gain;

    delay2=delay1;
    delay1=(int) y;
    phi1+=currDeltaPhi1;
    currDeltaPhi1+=d2Phi1;
    linAm1+=dA1;
    out[i]=in[i] + y*two_to_m23;
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
    int sint=sine_lut(phi0+mint);
    float gain=op0->mEnvelope.ProcessSample()*linAm0;
    float y=sint*gain; // y is now scaled by a factor of 2^23

    phi0+=currDeltaPhi0;
    currDeltaPhi0+=d2Phi0;
    linAm0+=dA0;

    // second operator, op1
    mint=((int) (y*two_to_7))<<3;
    sint=sine_lut(phi1+mint);
    gain=op1->mEnvelope.ProcessSample()*linAm1;
    y=sint*gain;
    
    phi1+=currDeltaPhi1;
    currDeltaPhi1+=d2Phi1;
    linAm1+=dA1;

    // third operator, op2
    mint=((int) (y*two_to_7))<<3;
    sint=sine_lut(phi2+mint);
    gain=op2->mEnvelope.ProcessSample()*linAm2;
    y=sint*gain;
    
    delay2=delay1;
    delay1=(int) y;
    phi2+=currDeltaPhi2;
    currDeltaPhi2+=d2Phi2;
    linAm2+=dA2;
    out[i]=in[i] + (y*two_to_m23);
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
