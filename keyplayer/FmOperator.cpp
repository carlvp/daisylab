#include "keyplayer.h"
#include "FmOperator.h"
#include "Q23.h"

void FmOperator::Init(float sampleRate) {
  mEnv.Init(sampleRate);
  mOsc.Init(sampleRate);
}
  
void FmOperator::noteOn(const FmOperatorParam *p, float freq, unsigned vel) {
  mParam=p;
  mGate=true;
  mOsc.SetWaveform(mOsc.WAVE_SIN);
  // oscillator mode (fixed frequency or ratio)
  freq=(p->fixedFreq)? p->freq : p->freq*freq;
  mOsc.SetFreq(freq);
  mEnv.SetAttackTime(p->attack);
  mEnv.SetDecayTime(p->decay);
  mEnv.SetSustainLevel(p->sustain);
  mEnv.SetReleaseTime(p->release);
  mEnv.Retrigger(true);
}

void FmOperator::noteOff() {
  mGate=false;
}

void FmOperator::fillBuffer(std::int32_t *out,
			    const std::int32_t *in,
			    const std::int32_t *mod) {
  float totalLevel=mParam->totalLevel;
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float gain=mEnv.Process(mGate)*totalLevel;
    
    mOsc.SetAmp(gain);
    out[i]=in[i] + Q23::fromFloat(mOsc.Process());
  }
}

  
