#include "keyplayer.h"
#include "FmAlgorithm.h"
#include "FmOperator.h"
#include "Voice.h"

static const float zeroBuffer[BLOCK_SIZE] = {0};
static float tempBuffer1[BLOCK_SIZE];

#define BITSET1(a)           (1<<a)
#define BITSET2(a,b)         (BITSET1(a)     | BITSET1(b))
#define BITSET3(a,b,c)       (BITSET2(a,b)   | BITSET1(c))
#define BITSET4(a,b,c,d)     (BITSET2(a,b)   | BITSET2(c,d))
#define BITSET5(a,b,c,d,e)   (BITSET3(a,b,c) | BITSET2(d,e))
#define BITSET6(a,b,c,d,e,f) (BITSET3(a,b,c) | BITSET3(d,e,f))

//     0*
//     |
// 4 2 1
// | |/
// 5 3
// +-+

class FmAlgorithm7 : public FmAlgorithm {
public:
  FmAlgorithm7()
    : FmAlgorithm{2, BITSET2(3,5)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback);
    op[1].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[2].fillBuffer(tmp, tmp,  zero, pitchMod, 0);
    op[3].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm7 alg7;

// 5 4 3 2 1 0*
// | | | | | |
// +-+-+-+-+-+

class FmAlgorithm32 : public FmAlgorithm {
public:
  FmAlgorithm32()
    : FmAlgorithm{6, BITSET6(0,1,2,3,4,5)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    op[0].fillBuffer(out, in, zeroBuffer, pitchMod, feedback);
    for (unsigned i=1; i<6; ++i)
      op[i].fillBuffer(out, out, zeroBuffer, pitchMod, 0);
  }

};

static FmAlgorithm32 alg32;

const FmAlgorithm* FmAlgorithm::getAlgorithm(unsigned algorithmNumber) {
  switch (algorithmNumber) {
  case 7: return &alg7;
  case 32:
  default:
    return &alg32;
  }
}
