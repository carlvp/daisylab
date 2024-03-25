#include "keyplayer.h"
#include "FmAlgorithm.h"
#include "FmOperator.h"
#include "Voice.h"

static const float zeroBuffer[BLOCK_SIZE] = {0};
static float tempBuffer1[BLOCK_SIZE];
static float tempBuffer2[BLOCK_SIZE];

#define BITSET1(a)           (1<<a)
#define BITSET2(a,b)         (BITSET1(a)     | BITSET1(b))
#define BITSET3(a,b,c)       (BITSET2(a,b)   | BITSET1(c))
#define BITSET4(a,b,c,d)     (BITSET2(a,b)   | BITSET2(c,d))
#define BITSET5(a,b,c,d,e)   (BITSET3(a,b,c) | BITSET2(d,e))
#define BITSET6(a,b,c,d,e,f) (BITSET3(a,b,c) | BITSET3(d,e,f))

// 4 2 0*
// | | |
// 5 3 1
// +-+-+
class FmAlgorithm5 : public FmAlgorithm {
public:
  FmAlgorithm5()
    : FmAlgorithm{3, BITSET3(1,3,5)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback);
    op[1].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[2].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[3].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm5 alg5;

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

//   2 0*    2 0
//   | |     | |
// 4 3 1  *4 3 1
//  \|/     \|/
//   5       5
//   +       +
//
//  #16     #17
class FmAlgorithm16and17: public FmAlgorithm {
public:
  explicit FmAlgorithm16and17(int algo)
    : FmAlgorithm{1, BITSET1(5)}, is16mask{(algo==16)? -1 : 0}
  { }

  int is16mask; // selects where to put the feedback

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp1=tempBuffer1;
    float *tmp2=tempBuffer2;
    const float *zero=zeroBuffer;

    op[0].fillBuffer(tmp1, zero, zero, pitchMod, feedback & is16mask);
    op[1].fillBuffer(tmp1, zero, tmp1, pitchMod, 0);
    op[2].fillBuffer(tmp2, zero, zero, pitchMod, 0);
    op[3].fillBuffer(tmp1, tmp1, tmp2, pitchMod, 0);
    op[4].fillBuffer(tmp1, tmp1, zero, pitchMod, feedback & ~is16mask);
    op[5].fillBuffer(out,  in,   tmp1, pitchMod, 0);
  }
};

static FmAlgorithm16and17 alg16{16}, alg17{17};

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
  case 5:  return &alg5;
  case 7:  return &alg7;
  case 16: return &alg16;
  case 17: return &alg17;
  case 32:
  default:
    return &alg32;
  }
}
