#pragma once
#ifndef KEYPLAYER_H

#include <daisy_seed.h>
#include "KeyplayerConfig.h"

extern daisy::DaisySeed DaisySeedHw;

class KeyPlayer {
 public:
  void Init();

  static constexpr int MidiKeyA4=69;

  // Phase increment, which corresponds to the given midi key
  // The 2PI phase range is mapped onto the range of 32-bit integers
  constexpr int midiToPhaseIncrement(int key) const {
    return mDeltaPhiA4*exp2f((key-MidiKeyA4)/12.0f);
  }

  // Phase increment, which corresponds to a frequency in Hz
  constexpr int freqToPhaseIncrement(float freq) const {
    return mDeltaPhi1Hz*freq;
  }
  
 private:
  float mDeltaPhi1Hz;
  float mDeltaPhiA4;
};

extern KeyPlayer theKeyPlayer;

void initVoices();
void startAudioPath();
void processAudioPath();
unsigned getAudioPathTimestamp();

class Voice;
Voice *findVoice(unsigned channel, unsigned key);
Voice *allocateVoice(unsigned channel, unsigned key);

#endif
