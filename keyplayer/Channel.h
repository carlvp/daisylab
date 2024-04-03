#pragma once
#ifndef Channel_H
#define Channel_H

#include "configuration.h"

class Program;
class Voice;

class Channel {
 public:
  Channel()
    : mMasterVolume{1.0f}, mNumVoices{0}
  {
    reset();
  }

  void reset();

  void addVoice(Voice *v);
  void removeVoice(Voice *v);

  // Produce interleaved stereo mix of voices
  const float *mixVoices(float *stereoOut, const float *stereoIn) const {
    if (mNumVoices!=0) {
      mixVoicesPrivate(stereoOut, stereoIn);
      return stereoOut;
    }
    else
      return stereoIn;
  }

  static const float *mixAllChannels(float *stereoOut, const float *stereoIn) {
    for (Channel &ch: allChannels)
      stereoIn = ch.mixVoices(stereoOut, stereoIn);
    return stereoIn;
  }
  
  const Program *getProgram() const { return mProgram; }
  void setProgram(unsigned pgm);

  // Master volume [0, 1.0]
  void setMasterVolume(float v) {
    mMasterVolume=v;
    updateGain();
  }
  
  // Channel Volume [0,16383]
  void setChannelVolume(unsigned v) {
    float r=v/16384.0f;
    mChannelVolume=r*r;
    updateGain();
  }

  // Expression [0,16383]
  void setExpression(unsigned x) {
    mExpression=x/16384.0f;
    updateGain();
  }

  // Pan [0,16383] 0=hard left, 8192=center, 16383=hard right
  void setPan(unsigned p);

  static Channel allChannels[NUM_CHANNELS];

 private:
  const Program *mProgram;
  float mMasterVolume, mChannelVolume, mExpression, mPanLeft, mPanRight;
  float mLeftGain, mRightGain;
  Voice *mVoice[NUM_VOICES];
  unsigned mNumVoices;
  
  void updateGain() {
    float volume=mMasterVolume*mChannelVolume*mExpression;
    mLeftGain=volume*mPanLeft;
    mRightGain=volume*mPanRight;
  }

  void mixVoicesPrivate(float *stereoOut, const float *stereoIn) const;
};

#endif
