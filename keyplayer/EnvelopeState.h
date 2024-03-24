#pragma once
#ifndef EnvelopeState_H
#define EnvelopeState_H

#include "KeyplayerConfig.h"

class EnvelopeParam;

class EnvelopeState {
 public:
  float ProcessSample() {
    float env=mLevel+mVarying;
    mVarying*=mDecay;
    return env;
  }
  
  void noteOn(const EnvelopeParam *param,
	      float levelScaling,
	      float rateScaling);

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
  unsigned mStage, mBlocksLeft;
  float mLevel;
  float mVarying;
  float mDecay;
  float mLevelScaling, mRateScaling;
  void initStage(unsigned stage, const EnvelopeParam *param);
};

#endif
