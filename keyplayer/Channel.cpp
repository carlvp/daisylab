#include <cmath>
#include "AudioPath.h"
#include "Channel.h"
#include "Program.h"
#include "Voice.h"

static float monoMix[BLOCK_SIZE];

void Channel::reset() {
  setProgram(1);
  mExpression=16383/16384.0f;
  setChannelVolume(90*128);
  setPan(8192);
}

void Channel::addVoice(Voice *v) {
  mVoice[mNumVoices]=v;
  mNumVoices++;
}

void Channel::removeVoice(Voice *v) {
  unsigned i;

  for (i=0; i<mNumVoices; ++i)
    if (mVoice[i]==v) {
      mNumVoices--;
      mVoice[i]=mVoice[mNumVoices];
      break;
    }
}

Voice *Channel::findVoice(unsigned key) const {
  for (unsigned i=0; i<mNumVoices; ++i)
    if (mVoice[i]->getKey()==key)
      return mVoice[i];
  
  return nullptr;
}

void Channel::mixVoicesPrivate(float *stereoMix, const float *stereoIn) const {
  unsigned i;
  const float *monoIn=zeroBuffer;

  // Mix the channel's voices
  for (i=0; i<mNumVoices; ++i) {
    mVoice[i]->fillBuffer(monoMix, monoIn);
    monoIn=monoMix;
  }

  // Pan and add to the stero mix
  for (i=0; i<BLOCK_SIZE; ++i) {
    float x=monoMix[i];
    stereoMix[2*i] = stereoIn[2*i] + x*mLeftGain;
    stereoMix[2*i+1] = stereoIn[2*i+1] + x*mRightGain;
  }
}

void Channel::setProgram(unsigned program) {
  mProgram=Program::getProgram(program);
}

void Channel::setPan(unsigned p) {
  // -6dB equal power pan law: mLeftGain^2 + mRightGain^2 = 1
  float theta=(M_PI_2/16383)*p;
  mPanLeft=cosf(theta);
  mPanRight=sinf(theta);
  updateGain();
}
