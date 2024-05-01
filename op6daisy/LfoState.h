#pragma once
#ifndef LfoState_H
#define LfoState_H

class LfoParam;

class LfoState {
 public:
  LfoState()
    : mPhi{0}, mDelayCounter{0}, mSeed{0}
  { }

  // Don't need these -avoid copy/assignment by misstake
  LfoState(const LfoState&) = delete;
  LfoState& operator=(const LfoState&) = delete;
  
  void sync(const LfoParam &param);
  
  float sampleAndUpdate(const LfoParam &param);
  
 private:
  int mPhi;
  unsigned short mDelayCounter;
  int mSeed;
};

#endif
