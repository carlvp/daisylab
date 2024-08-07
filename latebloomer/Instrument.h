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
  Instrument();

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
  void sysEx(const unsigned char *buffer, unsigned length);

  // Midi Key A4 (used for tuning)
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
  unsigned mBaseChannel;
  unsigned mCurrTimestamp;
  unsigned mSysExPtr;
  unsigned mWaitClearUnderrun;
  const float mDeltaPhi1Hz;
  const float mDeltaPhiA4;
  unsigned short mCurrCutoff;
  unsigned short mCurrReso;
  Channel mChannel[NUM_CHANNELS];
  Voice mVoice[NUM_VOICES];
  static constexpr unsigned SYSEX_BUFFER_SIZE=4104;
  unsigned char mSysExBuffer[SYSEX_BUFFER_SIZE];

  Voice *allocateVoice(unsigned ch, unsigned key);
};

// Instrument singleton
// "there's only one Op6Daisy and that is theOp6Daisy"
extern Instrument theOp6Daisy;

#endif
