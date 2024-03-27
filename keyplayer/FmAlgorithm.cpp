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

//    0*    0
//    |     |
//    1     1
//    |     |
// 4  2  4* 2
// |  |  |  |
// 5  3  5  3
// +--+  +--+
//
//  #1    #2

class FmAlgorithm1and2: public FmAlgorithm {
public:
  explicit FmAlgorithm1and2(int algo)
    : FmAlgorithm{2, BITSET2(3,5)}, mask0{(algo==1)? -1 : 0}
  { }

  int mask0; // selects where to put the feedback

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    int mask4=~mask0;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask0);
    op[1].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[2].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[3].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask4);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm1and2 alg1{1}, alg2{2};

// 3  0*
// |  |
// 4  1
// |  |
// 5  2
// +--+

class FmAlgorithm3 : public FmAlgorithm {
public:
  FmAlgorithm3()
    : FmAlgorithm{2, BITSET2(2,5)}
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
    op[2].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[3].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[4].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm3 alg3;

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

//     0*      0      0
//     |       |      |
// 4 2 1  4 *2 1 *4 2 1
// | |/   |  |/   | |/
// 5 3    5  3    5 3
// +-+    +--+    +-+
//
//  #7     #8     #9

class FmAlgorithm789 : public FmAlgorithm {
public:
  FmAlgorithm789(int algo)
    : FmAlgorithm{2, BITSET2(3,5)},
      mask0{(algo==7)? -1 : 0},
      mask2{(algo==8)? -1 : 0}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    int mask4=~(mask0 | mask2);
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask0);
    op[1].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[2].fillBuffer(tmp, tmp,  zero, pitchMod, feedback & mask2);
    op[3].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask4);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }

  int mask0, mask2;
};

static FmAlgorithm789 alg7{7}, alg8{8}, alg9{9};

// 3*       3
// |        |
// 4 1   0  4 1   0*
// |  \ /   |  \ /
// 5   2    5   2
// +---+    +---+
//
//  #10      #11

class FmAlgorithm10and11: public FmAlgorithm {
public:
  explicit FmAlgorithm10and11(int algo)
    : FmAlgorithm{2, BITSET2(2,5)}, mask0{(algo==11)? -1 : 0}
  { }

  int mask0; // selects where to put the feedback

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    int mask3=~mask0;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask0);
    op[1].fillBuffer(tmp, tmp,  zero, pitchMod, 0);
    op[2].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[3].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask3);
    op[4].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm10and11 alg10{10}, alg11{11};

// 4* 2 1 0  4  2 1 0*
// |   \|/   |   \|/
// 5    3    5    3
// +----+    +----+
//
//  #12       #13

class FmAlgorithm12and13: public FmAlgorithm {
public:
  explicit FmAlgorithm12and13(int algo)
    : FmAlgorithm{2, BITSET2(3,5)}, mask0{(algo==13)? -1 : 0}
  { }

  int mask0; // selects where to put the feedback

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    int mask4=~mask0;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask0);
    op[1].fillBuffer(tmp, tmp,  zero, pitchMod, 0);
    op[2].fillBuffer(tmp, tmp,  zero, pitchMod, 0);
    op[3].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask4);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm12and13 alg12{12}, alg13{13};


//  1 0*   1 0
//   \|     \|
//  4 2   *4 2
//  | |    | |
//  5 3    5 3
//  +-+    +-+
//
//  #14    #15

class FmAlgorithm14and15: public FmAlgorithm {
public:
  explicit FmAlgorithm14and15(int algo)
    : FmAlgorithm{2, BITSET2(3,5)}, mask0{(algo==14)? -1 : 0}
  { }

  int mask0; // selects where to put the feedback

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    int mask4=~mask0;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask0);
    op[1].fillBuffer(tmp, tmp,  zero, pitchMod, 0);
    op[2].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[3].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, feedback & mask4);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm14and15 alg14{14}, alg15{15};

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
    : FmAlgorithm{1, BITSET1(5)}, mask0{(algo==16)? -1 : 0}
  { }

  int mask0; // selects where to put the feedback

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp1=tempBuffer1;
    float *tmp2=tempBuffer2;
    const float *zero=zeroBuffer;
    int mask4=~mask0;
    
    op[0].fillBuffer(tmp1, zero, zero, pitchMod, feedback & mask0);
    op[1].fillBuffer(tmp1, zero, tmp1, pitchMod, 0);
    op[2].fillBuffer(tmp2, zero, zero, pitchMod, 0);
    op[3].fillBuffer(tmp1, tmp1, tmp2, pitchMod, 0);
    op[4].fillBuffer(tmp1, tmp1, zero, pitchMod, feedback & mask4);
    op[5].fillBuffer(out,  in,   tmp1, pitchMod, 0);
  }
};

static FmAlgorithm16and17 alg16{16}, alg17{17};

//     0
//     |
//     1
//   3*|
// 4 | 2
//  \|/
//   5
//   +

class FmAlgorithm18 : public FmAlgorithm {
public:
  FmAlgorithm18()
    : FmAlgorithm{1, BITSET1(5)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[1].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[2].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[3].fillBuffer(tmp, tmp,  zero, pitchMod, feedback);
    op[4].fillBuffer(tmp, tmp,  zero, pitchMod, 0);
    op[5].fillBuffer(out, in,   tmp,  pitchMod, 0);
  }
};

static FmAlgorithm18 alg18;

// 3
// |
// 4   0*
// |  /|
// 5 2 1
// +-+-+

class FmAlgorithm19 : public FmAlgorithm {
public:
  FmAlgorithm19()
    : FmAlgorithm{3, BITSET3(1,2,5)}
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
    op[2].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[3].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[4].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm19 alg19;

// *3 1 0
//  |\ \|
//  5 4 2
//  +-+-+

class FmAlgorithm20 : public FmAlgorithm {
public:
  FmAlgorithm20()
    : FmAlgorithm{3, BITSET3(2,4,5)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[1].fillBuffer(tmp, tmp,  zero, pitchMod, 0);
    op[2].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[3].fillBuffer(tmp, zero, zero, pitchMod, feedback);
    op[4].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm20 alg20;

// 3*    0    3   0*
// |\   /|    |  /|
// 5 4 2 1  5 4 2 1
// +-+-+-+  +-+-+-+
//
//   #21      #23

class FmAlgorithm21and23 : public FmAlgorithm {
public:
  explicit FmAlgorithm21and23(int n)
	  : FmAlgorithm{4, BITSET4(1,2,4,5)}, is23{(n==23)? -1 : 0}
  { }

	int is23; // selects where to put the feedback and how to route [3]
	
  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, is23 & feedback);
    op[1].fillBuffer(out, in,   tmp,  pitchMod, 0);
    op[2].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[3].fillBuffer(tmp, zero, zero, pitchMod, (~is23) & feedback);
    op[4].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[5].fillBuffer(out, out,  is23? zero : tmp,  pitchMod, 0);
  }
};

static FmAlgorithm21and23 alg21{21}, alg23{23};

// 
// 4   0*
// |  /|\ :-)
// 5 3 2 1
// +-+-+-+

class FmAlgorithm22 : public FmAlgorithm {
public:
  FmAlgorithm22()
    : FmAlgorithm{4, BITSET4(1,2,3,5)}
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
    op[2].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[3].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm22 alg22;

//       0*         0*           0*
//      /|\         |\           | 
// 5 4 3 2 1  5 4 3 2 1  5 4 3 2 1
// +-+-+-+-+  +-+-+-+-+  +-+-+-+-+
//
//    #24        #25        #31     

class FmAlgorithm24_25and31 : public FmAlgorithm {
public:
  explicit FmAlgorithm24_25and31(int n)
	  : FmAlgorithm{5, BITSET5(1,2,3,4,5)},
	    is24{n==24},
	    is31{n==31}
	{ }

	bool is24, is31; // selects how to route [0]
	
  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    
    op[0].fillBuffer(tmp, zero, zero, pitchMod, feedback);
    op[1].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[2].fillBuffer(out, out,  is31? zero : tmp, pitchMod, 0);
    op[3].fillBuffer(out, out,  is24? tmp : zero, pitchMod, 0);
    op[4].fillBuffer(out, out,  zero, pitchMod, 0);
    op[5].fillBuffer(out, out,  zero, pitchMod, 0);
  }
};

static FmAlgorithm24_25and31 alg24{24}, alg25{25}, alg31{31};

//   3 1 0*  *3 1 0
//   |  \|    |  \|
// 5 4   2  5 4   2
// +-+---+  +-+---+
//
//   #26      #27

class FmAlgorithm26and27: public FmAlgorithm {
public:
  explicit FmAlgorithm26and27(int algo)
	  : FmAlgorithm{1, BITSET3(2,4,5)}, mask0{(algo==26)? -1 : 0}
  { }

  int mask0; // selects where to put the feedback

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
    float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;
    int mask3=~mask0;
    
    op[0].fillBuffer(tmp,  zero, zero, pitchMod, feedback & mask0);
    op[1].fillBuffer(tmp,  tmp,  zero, pitchMod, 0);
    op[2].fillBuffer(out,  in,   tmp, pitchMod, 0);
    op[3].fillBuffer(tmp,  zero, zero, pitchMod, feedback & mask3);
    op[4].fillBuffer(out,  out,  tmp, pitchMod, 0);
    op[5].fillBuffer(out,  out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm26and27 alg26{26}, alg27{27};

//   1*
//   |
// 4 2
// | |
// 5 3 0
// +-+-+

class FmAlgorithm28 : public FmAlgorithm {
public:
  FmAlgorithm28()
    : FmAlgorithm{3, BITSET3(0,3,5)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
	  float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;

    op[0].fillBuffer(out, in,   zero, pitchMod, 0);
    op[1].fillBuffer(tmp, zero, zero, pitchMod, feedback);
    op[2].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[3].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[4].fillBuffer(tmp, zero, zero, pitchMod, 0);
    op[5].fillBuffer(out, out,  tmp,  pitchMod, 0);
  }
};

static FmAlgorithm28 alg28;

//     2 0*
//     | |
// 5 4 3 1
// +-+-+-+

class FmAlgorithm29 : public FmAlgorithm {
public:
  FmAlgorithm29()
	  : FmAlgorithm{4, BITSET4(1,3,4,5)}
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
    op[4].fillBuffer(out, out,  zero, pitchMod, 0);
    op[5].fillBuffer(out, out,  zero, pitchMod, 0);
  }
};

static FmAlgorithm29 alg29;

//     1*
//     |
//     2
//     | 
// 5 4 3 0
// +-+-+-+

class FmAlgorithm30 : public FmAlgorithm {
public:
  FmAlgorithm30()
	  : FmAlgorithm{4, BITSET4(0,3,4,5)}
  { }

  virtual void fillBuffer(float *out,
			  const float *in,
			  FmOperator *op,
			  float pitchMod,
			  unsigned feedback) const override {
	  float *tmp=tempBuffer1;
    const float *zero=zeroBuffer;

    op[0].fillBuffer(out, in,   zero, pitchMod, 0);
    op[1].fillBuffer(tmp, zero, zero, pitchMod, feedback);
    op[2].fillBuffer(tmp, zero, tmp,  pitchMod, 0);
    op[3].fillBuffer(out, out,  tmp,  pitchMod, 0);
    op[4].fillBuffer(out, out,  zero, pitchMod, 0);
    op[5].fillBuffer(out, out,  zero, pitchMod, 0);
  }
};

static FmAlgorithm30 alg30;

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
  case 1:  return &alg1;
  case 2:  return &alg2;
  case 3:  return &alg3;
  case 5:  return &alg5;
  case 7:  return &alg7;
  case 8:  return &alg8;
  case 9:  return &alg9;
  case 10: return &alg10;
  case 11: return &alg11;
  case 12: return &alg12;
  case 13: return &alg13;
  case 14: return &alg14;
  case 15: return &alg15;
  case 16: return &alg16;
  case 17: return &alg17;
  case 18: return &alg18;
  case 19: return &alg19;
  case 20: return &alg20;
  case 21: return &alg21;
  case 22: return &alg22;
  case 23: return &alg23;
  case 24: return &alg24;
  case 25: return &alg25;
  case 26: return &alg26;
  case 27: return &alg27;
  case 28: return &alg28;
  case 29: return &alg29;
  case 30: return &alg30;
  case 31: return &alg31;
  case 32:
  default:
    return &alg32;
  }
}
