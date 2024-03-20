#pragma once
#ifndef Voice_H
#define Voice_H

#include <daisysp.h>

#define NUM_VOICES 4

struct Program {
  const char *name;
  float attackTime;
  float decayTime;
};

class Voice {
 public:
 Voice()
   : mKey{0}, mNoteOn{false}, mTimestamp{0}, mProgram{nullptr}
    { }

  void Init(float sampleRate);

  unsigned getKey() const { return mKey; }

  bool isNoteOn() const { return mNoteOn; }

  // Voice stealing is based on note-on status and "age"
  Voice *voiceStealing(Voice *other, unsigned currTimestamp) {
    unsigned thisAge=currTimestamp-mTimestamp;
    unsigned otherAge=currTimestamp-other->mTimestamp;

    if (other->mNoteOn) {
      return (!mNoteOn || thisAge>=otherAge)? this : other;
    }
    else {
      return (!mNoteOn && thisAge>=otherAge)? this : other;
    }
  }
  
  void setProgram(const Program *p) {
    mProgram=p;
  }
  
  void noteOn(unsigned key, unsigned velocity);

  void noteOff();

  void addToBuffer(float *buf);

 private:
  unsigned char mKey;
  bool mNoteOn;
  unsigned mTimestamp;
  const Program *mProgram;
  daisysp::Oscillator mOsc;
  daisysp::AdEnv mEnv;
};

extern Voice allVoices[NUM_VOICES];

#endif
