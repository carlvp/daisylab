#pragma once
#ifndef EnvelopeState_H
#define EnvelopeState_H

#include "configuration.h"

class EnvelopeParam;

class EnvelopeState {
 public:
  explicit EnvelopeState(bool samplePerBlock)
    : mSamplePerBlock{samplePerBlock}
  { }

  float ProcessSample() {
    float env=mLevel+mVarying;
    mVarying*=mDecay;
    return env;
  }
  
  void noteOn(const EnvelopeParam *param,
	      float levelScaling,
	      float timeScaling);

  void noteOff(const EnvelopeParam *param) {
    if (mStage<RELEASE_STAGE)
      initStage(RELEASE_STAGE, param);
  }

  void updateAfterBlock(const EnvelopeParam *param) {
    if (--mBlocksLeft==0) {
      unsigned nextStage = mStage+1;

      if (nextStage<NUM_ENV_STAGES && nextStage!=RELEASE_STAGE)
	initStage(nextStage, param);
    }
  }
  
 private:
  bool mSamplePerBlock;
  unsigned char mStage;
  unsigned mBlocksLeft;
  float mLevel;
  float mVarying;
  float mDecay;
  float mLevelScaling, mTimeScaling;
  void initStage(unsigned stage, const EnvelopeParam *param);
};

#endif
