#pragma once
#ifndef FmOperator_H
#define FmOperator_H

#include <cstdint>
#include "EnvelopeState.h"

class FmOperatorParam;

class FmOperator {
 public:
  void noteOn(const FmOperatorParam *p,
	      unsigned key,
	      unsigned velocity,
	      std::int32_t deltaPhi,
	      float levelCom);

  void noteOff(const FmOperatorParam *param);

  void handleAm(float gain, float lfo);
  
  void fillBuffer(float *out,
		  const float *in,
		  const float *mod,
		  float pitchMod,
		  float ampMod,
		  int feedback);

  // Two-operator feedback
  // op[] is an array of operators op[0]->op[1]->out (and back to op[0])
  // There is no 'mod' input -otherwise like the fillBuffer member function
  static void fillBufferFb2(FmOperator op[],
			    float *out,
			    const float *in,
			    float pitchMod,
			    float ampMod,
			    int feedback);

  // Three-operator feedback
  // op[] is an array of operators op[0]->op[1]->op[2]->out (and back again)
  // There is no 'mod' input -otherwise like the fillBuffer member function
  static void fillBufferFb3(FmOperator op[],
			    float *out,
			    const float *in,
			    float pitchMod,
			    float ampMod,
			    int feedback);

 private:
  const FmOperatorParam *mParam;
  unsigned mPhi{0};
  int mDeltaPhiKey, mCurrDeltaPhi;
  int mDelay1, mDelay2;
  float mCurrAm;
  EnvelopeState mEnvelope{false}; // false=sample envelope at audio rate
};

#endif
