#pragma once
#ifndef DelayFx_H
#define DelayFx_H

#include<stdint.h>

#include "OnePoleLowPass.h"

class DelayFx {
 public:
  // Samples are stored as 16-bit int.
  // Size is measured in units of Blocks (see configuration.h)
  DelayFx(int16_t *buffer, unsigned numBlocks)
  : mBufferBegin{buffer},
    mBufferEnd{buffer+numBlocks*BLOCK_SIZE},
    mBufferFront{buffer},
    mBufferBack{buffer},
    mFeedback{0}
  {
    clearBuffer(); // sets mEndValidSamples
  }

  // clear buffer (in a sneaky way)
  void clearBuffer();
  
  // Set delay in ms (min 1 block, max determined by buffer size)
  void setDelayMs(unsigned ms);

  // Set feedback [0, 0.999]
  void setFeedback(float feedback) {
    mFeedback=feedback;
  }

  // Set high-frequency damping/cutoff (Hz)
  void setDamping(float hz) {
    mDampingFilter.setCoefficient(OnePoleLowpass::hz2Coefficient(hz));
  }

  // process one block of samples [-1.0, +1.0]
  // stereoOutput   Block of samples, with which the output of the delay line
  //                is mixed (thus input and output). Interleaved stereo samples
  // delayBusInput  Block of samples to feed into the delay line
  void processBlock(float *mixWithOutput, const float *delayBusInput);

 private:
  int16_t *mBufferBegin;
  int16_t *mBufferEnd;
  const int16_t *mBufferFront;
  int16_t *mBufferBack;
  int16_t *mEndValidSamples;
  float mFeedback;
  OnePoleLowpass mDampingFilter;
};

#endif
