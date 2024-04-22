#pragma once
#ifndef Instrument_H
#define Instrument_H

#include "configuration.h"
#include "Channel.h"
#include "Program.h"
#include "Voice.h"

struct SyxBulkFormat;

class Instrument {
 public:
  Instrument() = default;
  
  // Instrument is used as a singleton, we don't ever want to copy it
  Instrument(const Instrument&) = delete;
  Instrument& operator=(const Instrument&) = delete;

  void Init();

  void fillBuffer(float *stereoBuffer);
  
  // channel: 0-15, key: 0-127, velocity: 0-127
  void noteOn(unsigned channel, unsigned key, unsigned velocity);
  
  // channel: 0-15, key: 0-127
  void noteOff(unsigned channel, unsigned key);

  // channel: 0-15, cc: 0-127, value: 0-127
  void controlChange(unsigned channel, unsigned cc, unsigned value);
  
  // channel: 0-15, program: 0-127
  void programChange(unsigned channel, unsigned program);

  // channel: 0-15, value: -8192..0..+8191
  void pitchBend(unsigned channel, int value);

  // system exclusive
  void sysEx(unsigned char *buffer, unsigned length);

 private:
  unsigned mCurrTimestamp{0};
  Channel mChannel[NUM_CHANNELS];
  Voice mVoice[NUM_VOICES];
  Program mProgram[NUM_PROGRAMS];
  static constexpr unsigned SYSEX_BUFFER_SIZE=4104;
  unsigned char mSysExBuffer[SYSEX_BUFFER_SIZE];
  unsigned mSysExPtr{0};
  
  Voice *allocateVoice(unsigned ch, unsigned key);
  void loadSyxBulkFormat(const SyxBulkFormat *syx);
};

#endif
