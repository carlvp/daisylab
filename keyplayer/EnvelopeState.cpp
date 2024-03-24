#include <cmath>

#include "Program.h"
#include "EnvelopeState.h"

void EnvelopeState::noteOn(const EnvelopeParam *param,
			   float levelScaling,
			   float rateScaling) {
  mLevelScaling=levelScaling;
  mRateScaling=rateScaling;
  mLevel=param->level0*levelScaling;
  mVarying=0;
  initStage(0, param);
}

void EnvelopeState::initStage(unsigned stage, const EnvelopeParam *param) {
  unsigned numSamples=param->times[stage]*SAMPLE_RATE*mRateScaling;
  
  mStage=stage;
  mBlocksLeft=1 + numSamples/BLOCK_SIZE;

  // Envelope time is (a bit arbitrarily) defined as the time takes for
  // the amplitude to decay to exp(-PI), which is about -27dB.
  // Hence the "magic constant" PI*LOG2(e)=4.532360
  mDecay=exp2f(-4.53236f/numSamples);
  
  // env = (mLevel + mVarying). For a smooth transition the varying needs
  // to account for the difference in levels
  float newLevel=param->levels[stage]*mLevelScaling;
  mVarying += mLevel - newLevel;
  mLevel = newLevel;
}
