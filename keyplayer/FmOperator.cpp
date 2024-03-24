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
			    unsigned feedback) {
  unsigned phi=mPhi;
  
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float s=sinf(ldexpf((float) M_PI*phi,-31));
    float gain=mEnvelope.ProcessSample();
    
    phi+=mDeltaPhiKey;
    out[i]=in[i] + s*gain;
  }

  // Write back updated state
  mPhi=phi;
  mEnvelope.updateAfterBlock(&mParam->envelope);
}
