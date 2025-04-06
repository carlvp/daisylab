#pragma once
#ifndef Channel_H
#define Channel_H

#include "configuration.h"
#include "LfoState.h"

class Program;
class Voice;

class Channel {
 public:
  Channel()
    : mMasterVolume{0.5f}, mNumVoices{0}
  {
  }

  // Avoid accidental usage
  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;
  
  void reset(const Program *program);

  void addVoice(Voice *v);
  void removeVoice(Voice *v);


  // search for a voice, which is associated with this key
  Voice *findVoice(unsigned key) const;

  // search for a voice to allocate for a given key
  // a) in monophonic mode (there can be only one): any voice will do
  // b) in polyphonic, portamento mode: the given key is preferred
  //    but any voice, which is associated with a released key will do
  //    TODO: polyphonic portamento 
  // c) otherwise just the given key matches (same as findVoice)
  // nullptr is retured if no Voice was found
  Voice *allocateVoice(unsigned key) const {
    if (mPoly)
      return findVoice(key);
    else
      return (mNumVoices==0)? nullptr : mVoice[0];
  }

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

  // poly=true (polyphonic), false (monophonic)
  void setPoly(bool poly) { mPoly=poly; }

  // Portamento Time [0,16383]
  void setPortamentoTime(unsigned t);

  // Glide decay factor [0, 1.0)
  float getGlideDecayFactor() const { return mGlideDecayFactor; }

 private:
  const Program *mProgram;
  float mMasterVolume, mChannelVolume, mExpression, mPanLeft, mPanRight;
  float mLeftGain, mRightGain;
  float mPitchBendFactor, mPitchBendRange, mGlideDecayFactor;
  LfoState mLfo;
  bool mPoly;
  Voice *mVoice[NUM_VOICES];
  unsigned mNumVoices;

  void updateGain() {
    float volume=mMasterVolume*mChannelVolume*mExpression;
    mLeftGain=volume*mPanLeft;
    mRightGain=volume*mPanRight;
  }

  void mixVoicesPrivate(float *stereoOut, const float *stereoIn);
};

#endif
