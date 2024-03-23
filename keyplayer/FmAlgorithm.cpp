#include "keyplayer.h"
#include "FmAlgorithm.h"
#include "FmOperator.h"
#include "Voice.h"

static const float zeroBuffer[BLOCK_SIZE] = {0};

#define BITSET1(a)           (1<<a)
#define BITSET2(a,b)         (BITSET1(a)     | BITSET1(b))
#define BITSET3(a,b,c)       (BITSET2(a,b)   | BITSET1(c))
#define BITSET4(a,b,c,d)     (BITSET2(a,b)   | BITSET2(c,d))
#define BITSET5(a,b,c,d,e)   (BITSET3(a,b,c) | BITSET2(d,e))
#define BITSET6(a,b,c,d,e,f) (BITSET3(a,b,c) | BITSET3(d,e,f))

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
  return &alg32;
}
