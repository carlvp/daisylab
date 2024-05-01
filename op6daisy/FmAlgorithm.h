#pragma once
#ifndef Algorithm_H
#define Algorithm_H

class FmOperator;

class FmAlgorithm {
 public:
  FmAlgorithm(unsigned numOutputs, unsigned outputMask)
    : mNumOutputs{numOutputs}, mOutputMask{outputMask}
    {}

  // Algorithms are numbered 0..31
  static const FmAlgorithm* getAlgorithm(unsigned algorithmNumber);
  
  // Number of output operators in this algorithm 
  unsigned getNumOutputs() const {
    return mNumOutputs;
  }

  // Tests if operator op is an output.
  // N.B. Operators are numbered from zero, in the same order as in the mOp 
  // array of a Voice. This is also the order, in which operators are processed.
  // A source of confusion is that it's the reverse "Yamaha order", that is:
  // OP6 ~ 0, OP5 ~ 1, ..., OP1 ~ 5. Thus, isOutput(0) queries "OP6".
  bool isOutput(unsigned op) const {
    return ((1<<op) & mOutputMask) != 0;
  }

  // Produces samples in out[], mixed with in[], using the FmOperators in op[]
  // pitchMod specifies the pitch modulation (as a linear factor [0.5,2.0])
  // and feedback the feedback level (applies to feedback oscillators).
  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  float lfo,
			  unsigned feedback) const = 0;

 private:
  unsigned mNumOutputs, mOutputMask;
};

#endif
