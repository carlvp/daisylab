#pragma once
#ifndef Voice_H
#define Voice_H

#include <daisysp.h>

#include "keyplayer.h"
#include "FmOperator.h"

class Channel;
class FmAlgorithm;
struct Program;

class Voice {
 public:
 Voice()
   : mKey{0}, mGate{false}, mTimestamp{0}, mProgram{nullptr}
    { }

  void Init();

  unsigned getKey() const { return mKey; }

  bool isNoteOn() const { return mGate; }

  // Voice stealing is based on note-on status and "age"
  Voice *voiceStealing(Voice *other, unsigned currTimestamp) {
    unsigned thisAge=currTimestamp-mTimestamp;
    unsigned otherAge=currTimestamp-other->mTimestamp;

    if (other->mGate) {
      return (!mGate || thisAge>=otherAge)? this : other;
    }
    else {
      return (!mGate && thisAge>=otherAge)? this : other;
    }
  }
  
  void noteOn(Channel *ch, unsigned key, unsigned velocity);

  void noteOff();

  void fillBuffer(float *monoOut, const float *monoIn);

 private:
  unsigned char mKey;
  bool mGate;
  unsigned mTimestamp;
  Channel *mChannel;
  const Program *mProgram;
  const FmAlgorithm *mAlgorithm;
  FmOperator mOp[NUM_OPERATORS];
};

extern Voice allVoices[NUM_VOICES];

#endif
