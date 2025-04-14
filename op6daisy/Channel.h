#pragma once
#ifndef Channel_H
#define Channel_H

#include "configuration.h"
#include "LfoState.h"
#include "SetOfKeys.h"
#include "Voice.h"

class Program;

enum PortamentoMode {
  Off,
  Legato,  // portamento when notes played "legato" (overlapped)
  AlwaysOn
};

enum ModulationSource {
  ModWheel=0x01,
  FootPedal=0x02,
  BreathCtrl=0x04,
  ChannelPressure=0x08,
};

enum ModulationDestination {
  LfoPmDepth,
  PitchBend,
  LfoAmDepth,
  AmpBias,
  NUM_MODULATION_DESTINATIONS
};

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

  void noteOn(Voice *v, unsigned key, unsigned velocity, unsigned timestamp);

  void noteOff(unsigned key, unsigned timestamp);

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
  void setProgram(const Program *program);

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

  // Channel pressure [0, 127]
  void setChannelPressure(unsigned char value);

  // Channel pressure modulation range [0, 127]
  void setChannelPressureRange(unsigned char value);
  
  // Pitch bend [-8192, +8192]
  void setPitchBend(int b);

  // Pitch bend range (cents)
  void setPitchBendRange(unsigned cents);

  // poly=true (polyphonic), false (monophonic)
  void setPoly(bool poly) {
    mPoly=poly;
    allNotesOff();
  }

  // Portamento Time [0,16383]
  void setPortamentoTime(unsigned t);

  // Glide decay factor [0, 1.0)
  float getGlideDecayFactor() const { return mGlideDecayFactor; }

  // Portamento Mode
  void setPortamentoMode(PortamentoMode mode) {
    mPortamentoMode=static_cast<char>(mode);
  }

  PortamentoMode getPortamentoMode() const {
    return static_cast<PortamentoMode>(mPortamentoMode);
  }

  // set modulation wheel [0, 16383]
  void setModulationWheel(unsigned value) {
    mModWheel=value;
    updateModWheel(true);
  }

  // set modulation range [0, 16383]
  void setModulationRange(unsigned value) {
    mModulationRange=value;
    updateModWheel(true);
  }

  // set modulation routing (on/off)
  void setModulationRouting(ModulationSource src,
			    ModulationDestination dst,
			    bool routingEnabled);

 private:
  SetOfKeys mNotesOn;
  const Program *mProgram;
  float mMasterVolume, mChannelVolume, mExpression, mPanLeft, mPanRight;
  float mLeftGain, mRightGain;
  float mPitchBendInOctaves, mPitchBendRange, mGlideDecayFactor;
  unsigned mModulationRange, mModWheel;
  float mFromModWheel;
  float mLfoPmDepth, mLfoAmDepth, mAmpBias;
  LfoState mLfo;
  Voice *mVoice[NUM_VOICES];
  unsigned mNumVoices;
  short mPitchWheel;
  char mPortamentoMode, mLastKey;
  bool mPoly, mLastKeyUp;
  unsigned char mChPressure, mChPressureRange;
  unsigned char mModRouting[ModulationDestination::NUM_MODULATION_DESTINATIONS];

  void updateGain() {
    float volume=mMasterVolume*mChannelVolume*mExpression;
    mLeftGain=volume*mPanLeft;
    mRightGain=volume*mPanRight;
  }

  void mixVoicesPrivate(float *stereoOut, const float *stereoIn);
  Voice *findVoice(unsigned key) const;
  void addVoice(Voice *v);
  void removeVoice(Voice *v);
  void allNotesOff();
  void updateModWheel(bool propagate);
  void updateDestinations(ModulationSource sourceChanged);
  void updateLfoPmDepth();
  void updateLfoAmDepth();
  void updateAmpBias();
  void updatePitchBend();
};

#endif
