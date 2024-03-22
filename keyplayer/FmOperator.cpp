#include "keyplayer.h"
#include "FmOperator.h"

void FmOperator::Init(float sampleRate) {
  mEnv.Init(sampleRate);
  mOsc.Init(sampleRate);
}
  
void FmOperator::noteOn(const FmOperatorParam *p, float freq, unsigned vel) {
  mGate=true;
  mOsc.SetWaveform(mOsc.WAVE_SIN);
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

void FmOperator::fillBuffer(float *out, const float *in, const float *mod) {
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float gain=mEnv.Process(mGate);
    
    mOsc.SetAmp(gain);
    out[i]=in[i] + mOsc.Process();
  }
}

  
