#pragma once
#ifndef Instrument_H
#define Instrument_H

#include "configuration.h"
#include "Channel.h"
#include "DelayFx.h"
#include "Program.h"
#include "Voice.h"
#include "VoiceEditBuffer.h"

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

  // channel: 0-15, pressure 0-127
  void channelPressure(unsigned channel, unsigned program);
  
  // channel: 0-15, value: -8192..0..+8191
  void pitchBend(unsigned channel, int value);

  // system exclusive
  void sysEx(const unsigned char *buffer, unsigned length);

  // channel: 0-15
  void resetAllControllers(unsigned channel);

  // reset MIDI event
  void reset();

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
  enum HiresCC {
    DataEntry,
    NRPN,
    RPN,
    NUM_HIRES_CC
  };

  unsigned mBaseChannel;
  enum OperationalMode {
    kPerformanceMode,
    kEditMode,
    NUM_OPERATIONAL_MODES
  } mOperationalMode;
  unsigned mCurrTimestamp;
  unsigned mSysExPtr;
  unsigned mWaitClearUnderrun;
  const float mDeltaPhi1Hz;
  const float mDeltaPhiA4;
  Channel mChannel[NUM_CHANNELS];
  unsigned short mDataEntryRouting[NUM_CHANNELS];
  unsigned short mHiresControls[NUM_CHANNELS][HiresCC::NUM_HIRES_CC];
  Voice mVoice[NUM_VOICES];
  VoiceEditBuffer mVoiceEditBuffer;
  Program mProgram[NUM_PROGRAMS];
  Program mTempPrograms[NUM_VOICES];
  Program *mLastTempProgram;
  const Program *mSavedProgram;
  unsigned char mTempRefCount[NUM_VOICES];
  Program mEditBuffer;
  static constexpr unsigned SYSEX_BUFFER_SIZE=4104;
  unsigned char mSysExBuffer[SYSEX_BUFFER_SIZE];
  DelayFx mDelayFx;
  Mixer mMixer;

  bool isTempProgram(const Program *pgm) const {
    return mTempPrograms<=pgm && pgm<mTempPrograms+NUM_VOICES;
  }
  
  Program *getTempProgram(const Program *tryThisFirst);
  Program *releaseTempProgram(const Program *pgm);
  Voice *allocateVoice(unsigned ch, unsigned key);
  void loadSyxBulkFormat(const SyxBulkFormat *syx);
  void setParameter(unsigned ch, unsigned paramNumber, unsigned value);
  void setSystemParameter(unsigned parameterNumber, unsigned value);
  void setOperationalMode(unsigned mode);
  void setChannelParameter(unsigned ch, unsigned parameterNr, unsigned value);
  void controlChangeHires(unsigned ch, HiresCC cc, unsigned value);

  void controlChangeCoarse(unsigned ch, HiresCC cc, unsigned value) {
    // set most significant part of the high-resolution control
    controlChangeHires(ch, cc, value*128);
  }

  void controlChangeFine(unsigned ch, HiresCC cc, unsigned value) {
    // combine value with most significant part of the high-resolution control
    controlChangeHires(ch, cc, (mHiresControls[ch][cc] & 0xff80) | (value & 0x7f));
  }
};

// Instrument singleton
// "there's only one Op6Daisy and that is theOp6Daisy"
extern Instrument theOp6Daisy;

#endif
