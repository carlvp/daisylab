#ifndef MidiDispatcher_H
#define MidiDispatcher_H

#include <daisy_seed.h>

class UsbMidiDispatcher {
 public:
  void Init();

  void Process() {
    mMidi.Listen();
    if (mMidi.HasEvents())
      DispatchEvents();
  }
  
 private:
  daisy::MidiUsbHandler mMidi;
  
  void DispatchEvents();
  void noteOn(unsigned channel, unsigned key, unsigned velocity);
  void noteOff(unsigned channel, unsigned key);
  // channel: 0-15, cc: 0-127, value: 0-127
  void controlChange(unsigned channel, unsigned cc, unsigned value);
  void programChange(unsigned channel, unsigned program);
};

#endif
