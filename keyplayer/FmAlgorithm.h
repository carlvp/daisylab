#pragma once
#ifndef Algorithm_H
#define Algorithm_H

class FmOperator;

class FmAlgorithm {
 public:
  FmAlgorithm(unsigned numCarriers, unsigned carrierMask)
    : mNumCarriers{numCarriers}, mCarrierMask{carrierMask}
    {}

  // Algorithms are numbered 1..32
  static const FmAlgorithm* getAlgorithm(unsigned algorithmNumber);
  
  // Number of carriers (outputs) in this algorithm 
  unsigned getNumCarriers() const {
    return mNumCarriers;
  }

  // Tests if Operator op is a carrier (output)
  // N.B. Operators are numbered from zero, in the same order as in the mOp 
  // array of a Voice. This is also the order, in which operators are processed.
  // A source of confusion is that it's the reverse "Yamaha order", that is:
  // OP6 ~ 0, OP5 ~ 1, ..., OP1 ~ 5. Thus, isCarrier(0) queries "OP6".
  bool isCarrier(unsigned op) const {
    return ((1<<op) & mCarrierMask) != 0;
  }

  // Produces samples in out[], mixed with in[], using the FmOperators in op[]
  // pitchMod specifies the pitch modulation (as a linear factor [0.5,2.0])
  // and feedback the feedback level (applies to feedback oscillators).
  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const = 0;

 private:
  unsigned mNumCarriers, mCarrierMask;
};

#endif
