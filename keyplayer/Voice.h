#pragma once
#ifndef Voice_H
#define Voice_H

#include <daisysp.h>

#include "keyplayer.h"
#include "FmOperator.h"

struct Program {
  const char *name;
  unsigned char algorithm;
  FmOperatorParam op[NUM_OPERATORS];
};

class FmAlgorithm;

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
  
  void noteOn(const Program *p, unsigned key, unsigned velocity);

  void noteOff();

  void addToBuffer(float *buf);

 private:
  unsigned char mKey;
  bool mNoteOn;
  unsigned mTimestamp;
  const Program *mProgram;
  const FmAlgorithm *mAlgorithm;
  FmOperator mOp[NUM_OPERATORS];
};

extern Voice allVoices[NUM_VOICES];

#endif
