#ifndef MidiDispatcher_H
#define MidiDispatcher_H

#include "daisy_seed.h"

class UsbMidiDispatcher {
 public:
  void Init() {
    daisy::MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph = daisy::MidiUsbTransport::Config::INTERNAL;
    mMidi.Init(midi_cfg);
  }

  void Process() {
    mMidi.Listen();
    if (mMidi.HasEvents())
      DispatchEvents();
  }
  
 private:
  daisy::MidiUsbHandler mMidi;
  void DispatchEvents();
};

#endif
