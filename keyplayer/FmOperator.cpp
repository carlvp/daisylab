#include <cmath>
#include "keyplayer.h"
#include "FmOperator.h"
#include "Program.h"

void FmOperator::Init() {
   mPhi=0;
}
  
void FmOperator::noteOn(const FmOperatorParam *param,
			std::int32_t deltaPhi,
			unsigned vel,
			float levelCom) {
  mParam=param;
  // oscillator mode (fixed frequency or ratio)
  mDeltaPhiKey=(param->fixedFreq)?
    theKeyPlayer.freqToPhaseIncrement(param->freq)
    : param->freq*deltaPhi;

  // delays
  mDelay1=mDelay2=0;
  
  // envelope
  mEnvelope.noteOn(&param->envelope, param->totalLevel*levelCom, 1.0f);
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
