#pragma once
#ifndef FmOperator_H
#define FmOperator_H

#include <daisysp.h>

struct FmOperatorParam {
  float totalLevel;
  float attack, decay, sustain, release;
};

class FmOperator {
 public:
  void Init(float sampleRate);
  
  void noteOn(const FmOperatorParam *p, float freq, unsigned velocity);

  void noteOff();

  void fillBuffer(float *out, const float *in, const float *mod);
  
 private:
  const FmOperatorParam *p;
  bool mGate;
  daisysp::Oscillator mOsc;
  daisysp::Adsr mEnv;
};

#endif
