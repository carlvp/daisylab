#pragma once
#ifndef Voice_H
#define Voice_H

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

  unsigned getKey() const { return mKey; }

  bool isNoteOn() const { return mGate; }

  unsigned getTimestamp() const { return mTimestamp; }
  
  void noteOn(Channel *ch, unsigned key, unsigned velocity, unsigned timestamp);

  void noteOff(unsigned timestamp);

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

#endif
