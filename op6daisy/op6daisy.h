#pragma once
#ifndef OP6DAISY_H

class Op6Daisy {
 public:
  Op6Daisy();

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
  float mDeltaPhi1Hz;
  float mDeltaPhiA4;
};

extern Op6Daisy theOp6Daisy;

#endif
