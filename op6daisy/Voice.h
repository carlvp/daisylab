#pragma once
#ifndef Voice_H
#define Voice_H

#include "FmOperator.h"

class Channel;
class FmAlgorithm;
struct Program;

class Voice {
 public:
 Voice()
   : mKey{0}, mGate{false}, mTimestamp{0}, mChannel{nullptr}, mProgram{nullptr}
    { }

  unsigned getKey() const { return mKey; }
  bool isNoteOn() const { return mGate; }
  unsigned getTimestamp() const { return mTimestamp; }
  const Program *getProgram() const { return mProgram; }
  Channel *getChannel() const { return mChannel; }

  // start glide from a given offset in semi tones
  void setGlide(int slideOffset) {
    // glide offset in octaves (12 semis)
    mGlideCV += slideOffset*0.08333333f;
  }

  // change key without retriggering note (legato)
  void changeKey(unsigned key);
  
  void noteOn(Channel *ch,
	      unsigned key,
	      unsigned velocity,
	      bool retrig,
	      unsigned timestamp);

  void noteOff(unsigned timestamp);

  void kill();

  void fillBuffer(float *monoOut,
		  const float *monoIn,
		  float pitchMod,
		  float ampMod);

 private:
  unsigned char mKey;
  bool mGate;
  unsigned mTimestamp;
  Channel *mChannel;
  const Program *mProgram;
  const FmAlgorithm *mAlgorithm;
  float mGlideCV;
  EnvelopeState mEnvelope{true}; // true=sample envelope once per block
  FmOperator mOp[NUM_OPERATORS];
};

#endif
