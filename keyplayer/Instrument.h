#pragma once
#ifndef Instrument_H
#define Instrument_H

#include "configuration.h"
#include "Channel.h"
#include "Voice.h"

class Instrument {
 public:
  void fillBuffer(float *stereoBuffer);
  
  // channel: 0-15, key: 0-127, velocity: 0-127
  void noteOn(unsigned channel, unsigned key, unsigned velocity);
  
  // channel: 0-15, key: 0-127
  void noteOff(unsigned channel, unsigned key);

  // channel: 0-15, cc: 0-127, value: 0-127
  void controlChange(unsigned channel, unsigned cc, unsigned value);
  
  // channel: 0-15, program: 0-127
  void programChange(unsigned channel, unsigned program);
  
 private:
  unsigned mCurrTimestamp{0};
  Channel mChannel[NUM_CHANNELS];
  Voice mVoice[NUM_VOICES];

  Voice *allocateVoice(unsigned ch, unsigned key);
};

#endif
