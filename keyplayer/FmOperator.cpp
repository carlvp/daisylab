#include <cmath>
#include "keyplayer.h"
#include "FmOperator.h"
#include "Program.h"

void FmOperator::Init() {
   mPhi=0;
}

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

void FmOperator::noteOn(const FmOperatorParam *param,
			unsigned key,
			unsigned vel,
			std::int32_t deltaPhi,
			float levelCom) {
  mParam=param;
  // oscillator mode (fixed frequency or ratio)
  mDeltaPhiKey=(param->fixedFreq)?
    theKeyPlayer.freqToPhaseIncrement(param->freq)
    : param->freq*deltaPhi;

  // delays
  mDelay1=mDelay2=0;
  
  // envelope
  float level=param->totalLevel*levelCom;
  level*=velocityScaling(vel, param->velScaling);
  float timeS=timeScaling(key, param->kbdRateScaling);
  mEnvelope.noteOn(&param->envelope, level, timeS);
}

void FmOperator::noteOff(const FmOperatorParam *param) {
  mEnvelope.noteOff(&param->envelope);
}

void FmOperator::fillBuffer(float *out,
			    const float *in,
			    const float *mod,
			    float pitchMod,
			    int feedback) {
  unsigned phi=mPhi;
  float y1=mDelay1;
  float y2=mDelay2;
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    // Modulator is scaled by 4PI, which is the modulation index for ouput 99
    // Feedback is scaled similarly, but there is also an implicit
    // averaging of y1 and y2 and a scaling by 2^(feedback-9):
    // (2^k)PI, where k=2 {for 4PI} -1 {1/2 averaging} + feedback-9 =
    // = feedback-8.
    float m=(feedback)? ldexpf(y1+y2, feedback-8) : 4.0f*mod[i];
    float s=sinf((ldexpf(phi,-31)+m)*((float) M_PI));
    float gain=mEnvelope.ProcessSample();

    y2=y1;
    y1=s*gain;
    phi+=mDeltaPhiKey;
    out[i]=in[i] + y1;
  }

  // Write back updated state
  mPhi=phi;
  mDelay1=y1;
  mDelay2=y2;
  mEnvelope.updateAfterBlock(&mParam->envelope);
}
