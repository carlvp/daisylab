#include <cmath>
#include "LfoState.h"
#include "Program.h"

void LfoState::sync(const LfoParam &param) {
  mPhi=0;
  mDelayCounter=param.delay;
}

#define SIGN_BIT     0x80000000
#define BIT30        0x40000000
#define REST_OF_BITS 0x3fffffff

float LfoState::sampleAndUpdate(const LfoParam &param) {
  if (mDelayCounter) {
    --mDelayCounter;
    return 0;
  }

  // No fancy stuff to avoid aliasing. Max lfo speed is 55 Hz.
  int y=mPhi;
  int temp;
  mPhi+=param.deltaPhi;
  
  switch (param.waveform) {
  case WAVE_SINE:
    return sinf((y|1)*(M_PI/2147483648));
  case WAVE_SAW_UP: /* y = last mPhi */ break;
  case WAVE_SAW_DOWN: y=-(y|1); break;
  case WAVE_SQUARE: y=(y & SIGN_BIT)? 0x80000001 : 0x7fffffff; break;
  case WAVE_TRIANGLE:
    // mirror phase in second and fourth quadrants
    temp=(y & BIT30)? ~y : y;
    // negate result in third and fourth quadrants
    y=(y & SIGN_BIT)? -temp : temp;
    y<<=1;
    break;
  case WAVE_SAMPLE_HOLD:
    if (y<=0 && mPhi>0) {
      // Trigger sampling
      mSeed=1664525*mSeed + 1013904223;
    }
    y=mSeed;
    break;
  }

  return ldexpf(y|1,-31);
}
