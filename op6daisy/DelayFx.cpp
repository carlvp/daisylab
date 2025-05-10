#include<cmath>

#include "DelayFx.h"

static inline unsigned min(unsigned x, unsigned y) {
  return (x<y)? x : y;
}

void DelayFx::setDelayMs(unsigned ms) {
  unsigned numBlocks=(mBufferEnd-mBufferBegin)/BLOCK_SIZE;
  unsigned numBlocksDelay=(ms*(SAMPLE_RATE/BLOCK_SIZE)+500)/1000;
  unsigned numSamplesDelay=min(numBlocksDelay, numBlocks)*BLOCK_SIZE;
  int writePos=mBufferBack-mBufferBegin;
  int readPos=writePos-numSamplesDelay;

  mBufferFront=(readPos<0)? mBufferEnd+readPos : mBufferBegin+readPos;
}

static inline int clamp16bit(int x) {
  return (x<INT16_MIN)? INT16_MIN : (x>INT16_MAX)? INT16_MAX : x;
}

void DelayFx::processBlock(float *stereoMix, const float *delayBusInput) {

  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    // Produce output
    float x=ldexpf(*mBufferFront++, -15);
    float y=mDampingFilter.getNextSample(x);
    *(stereoMix++) += y;
    *(stereoMix++) += y;
    // store input (including feedback)
    float input=y*mFeedback + *(delayBusInput++);
    *(mBufferBack++)=clamp16bit(ldexpf(input, +15));
  }

  // Handle buffer wrap-around
  if (mBufferFront==mBufferEnd) mBufferFront=mBufferBegin;
  if (mBufferBack==mBufferEnd) mBufferBack=mBufferBegin;
}
