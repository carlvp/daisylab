#ifndef MidiDispatcher_H
#define MidiDispatcher_H

#include <daisy_seed.h>
#include "Channel.h"

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
  Channel mChannel[16];
  
  void DispatchEvents();
  void noteOn(unsigned channel, unsigned key, unsigned velocity);
  void noteOff(unsigned channel, unsigned key);
  void programChange(unsigned channel, unsigned program);
};

#endif
