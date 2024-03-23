#include "keyplayer.h"
#include "FmOperator.h"

void FmOperator::Init(float sampleRate) {
  mEnv.Init(sampleRate);
  mPhi=0;
}
  
void FmOperator::noteOn(const FmOperatorParam *p,
			std::int32_t deltaPhi,
			unsigned vel) {
  mParam=p;
  mGate=true;
  // oscillator mode (fixed frequency or ratio)
  mDeltaPhiKey=(p->fixedFreq)?
    theKeyPlayer.freqToPhaseIncrement(p->freq)
    : p->freq*deltaPhi;
  // envelope paramers
  mEnv.SetAttackTime(p->attack);
  mEnv.SetDecayTime(p->decay);
  mEnv.SetSustainLevel(p->sustain);
  mEnv.SetReleaseTime(p->release);
  mEnv.Retrigger(true);
}

void FmOperator::noteOff() {
  mGate=false;
}

void FmOperator::fillBuffer(float *out,
			    const float *in,
			    const float *mod,
			    float pitchMod,
			    unsigned feedback) {
  float totalLevel=mParam->totalLevel;
  unsigned phi=mPhi;
  
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float gain=mEnv.Process(mGate)*totalLevel;
    float s=sinf(ldexpf(PI_F*phi,-31));
    
    phi+=mDeltaPhiKey;
    out[i]=in[i] + s*gain;
  }

  // Write back updated state
  mPhi=phi;
}
