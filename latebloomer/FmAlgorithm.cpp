#include "AudioPath.h"
#include "FmAlgorithm.h"
#include "FmOperator.h"
#include "Voice.h"

static float tempBuffer1[BLOCK_SIZE];

#define BITSET1(a)           (1<<a)
#define BITSET2(a,b)         (BITSET1(a)     | BITSET1(b))
#define BITSET3(a,b,c)       (BITSET2(a,b)   | BITSET1(c))
#define BITSET4(a,b,c,d)     (BITSET2(a,b)   | BITSET2(c,d))
#define BITSET5(a,b,c,d,e)   (BITSET3(a,b,c) | BITSET2(d,e))
#define BITSET6(a,b,c,d,e,f) (BITSET3(a,b,c) | BITSET3(d,e,f))

//  1 0*
//   \|
//    2
//    |
//    3
//    
// Algorithm #2 of TX81z (DX11/21727/100)

class FmAlgorithm2: public FmAlgorithm {
public:
  explicit FmAlgorithm2()
    : FmAlgorithm{1, BITSET1(3)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  float lfo,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, lfo, feedback);
    op[1].fillBuffer(tmp, tmp,  zero, pitchMod, lfo, 0);
    op[2].fillBuffer(tmp, zero, tmp,  pitchMod, lfo, 0);
    op[3].fillBuffer(out, in,   tmp,  pitchMod, lfo, 0);
  }
};

static FmAlgorithm2 alg2;

const FmAlgorithm* FmAlgorithm::getAlgorithm(unsigned) {
  return &alg2;
}
