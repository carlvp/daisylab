#pragma once
#ifndef FmOperator_H
#define FmOperator_H

#include <cstdint>
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

  // Computes out[0:N-1] = in[0:N-1] + f(mod[0:N-1]), where N is the buffer size
  //
  // out[0:N-1] is the output buffer
  // in[0:N-1] is an input buffer which is mixed with the FmOperator's output
  // mod[0:N-1] is the phase modulation input (produced by other FmOperators)
  //
  // Samples are Q23 (signed fixed point, 23 fractional bits)
  void fillBuffer(std::int32_t *out,
		  const std::int32_t *in,
		  const std::int32_t *mod);

 private:
  const FmOperatorParam *mParam;
  bool mGate;
  unsigned mPhi, mDeltaPhiKey;
  daisysp::Adsr mEnv;
};

#endif
