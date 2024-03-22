#include "keyplayer.h"
#include "FmOperator.h"

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

void FmOperator::fillBuffer(float *out, const float *in, const float *mod) {
  float totalLevel=mParam->totalLevel;
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float gain=mEnv.Process(mGate)*totalLevel;
    
    mOsc.SetAmp(gain);
    out[i]=in[i] + mOsc.Process();
  }
}

  
