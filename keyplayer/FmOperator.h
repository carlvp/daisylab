#pragma once
#ifndef FmOperator_H
#define FmOperator_H

#include <daisysp.h>

struct FmOperatorParam {
  bool fixedFreq;
  float freq;              // Fixed frequency (Hz) or frequency ratio
  float totalLevel;
  float attack, decay, sustain, release;
};

class FmOperator {
 public:
  void Init(float sampleRate);
  
  void noteOn(const FmOperatorParam *p,
	      std::int32_t deltaPhi,
	      unsigned velocity);

  void noteOff();

  void fillBuffer(float *out, const float *in, const float *mod);
  
 private:
  const FmOperatorParam *mParam;
  bool mGate;
  unsigned mPhi, mDeltaPhiKey;
  daisysp::Adsr mEnv;
};

#endif
