#ifndef MidiDispatcher_H
#define MidiDispatcher_H

#include <daisy_seed.h>

class Instrument;

class UsbMidiDispatcher {
 public:
  void Init(Instrument *instrument);

  void Process() {
    mMidi.Listen();
    if (mMidi.HasEvents())
      DispatchEvents();
  }
  
 private:
  daisy::MidiUsbHandler mMidi;
  Instrument *mInstrument;

  void DispatchEvents();  
};

#endif
