#pragma once
#ifndef FmOperator_H
#define FmOperator_H

#include "EnvelopeState.h"

class FmOperatorParam;

class FmOperator {
 public:
  void Init();
  
  void noteOn(const FmOperatorParam *p,
	      unsigned key,
	      unsigned velocity,
	      std::int32_t deltaPhi,
	      float levelCom);

  void noteOff(const FmOperatorParam *param);

  void fillBuffer(float *out,
		  const float *in,
		  const float *mod,
		  float pitchMod,
		  int feedback);
  
 private:
  const FmOperatorParam *mParam;
  unsigned mPhi, mDeltaPhiKey;
  float mDelay1, mDelay2;
  EnvelopeState mEnvelope;
};

#endif
