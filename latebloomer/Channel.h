#pragma once
#ifndef Channel_H
#define Channel_H

#include <daisysp.h>
#include "configuration.h"
#include "LfoState.h"

class Program;
class Voice;

class Channel {
 public:
  Channel()
    : mMasterVolume{0.5f}, mNumVoices{0}
  {
    mFilter.Init(SAMPLE_RATE);
  }

  // Avoid accidental usage
  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;
  
  void reset(const Program *program);

  void addVoice(Voice *v);
  void removeVoice(Voice *v);

  Voice *findVoice(unsigned key) const;
  
  // Produce interleaved stereo mix of voices
  const float *mixVoices(float *stereoOut, const float *stereoIn) {
    if (mNumVoices!=0) {
      mixVoicesPrivate(stereoOut, stereoIn);
      return stereoOut;
    }
    else
      return stereoIn;
  }

  const Program *getProgram() const { return mProgram; }

  void setProgram(const Program *program) {
    mProgram=program;
  }

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

  // Pitch bend [-8192, +8192]
  void setPitchBend(int b);

  // Pitch bend range (cents)
  void setPitchBendRange(unsigned cents);

  // Filter cutoff [0,16383] (it's an exponential scale, i.e. not in Hz)
  void setFilterCutoff(unsigned v);
  
  // Filter resonance [0,16383]
  void setFilterResonance(unsigned v) {
    mFilter.SetRes(v/16384.0f);
  }
  
 private:
  const Program *mProgram;
  float mMasterVolume, mChannelVolume, mExpression, mPanLeft, mPanRight;
  float mLeftGain, mRightGain;
  float mPitchBendFactor, mPitchBendRange;
  LfoState mLfo;
  Voice *mVoice[NUM_VOICES];
  unsigned mNumVoices;
  daisysp::Svf mFilter;
  
  void updateGain() {
    float volume=mMasterVolume*mChannelVolume*mExpression;
    mLeftGain=volume*mPanLeft;
    mRightGain=volume*mPanRight;
  }

  void mixVoicesPrivate(float *stereoOut, const float *stereoIn);
};

#endif
