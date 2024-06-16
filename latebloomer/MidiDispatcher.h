#ifndef MidiDispatcher_H
#define MidiDispatcher_H

#include <daisy_seed.h>

class Instrument;

class BasicMidiDispatcher {
protected:
  void DispatchEvent(daisy::MidiEvent &msg);
  
  Instrument *mInstrument;
};
  
template<typename Transport>
class MidiDispatcher: public BasicMidiDispatcher {
 public:
  void Init(Instrument *instrument, daisy::MidiHandler<Transport> *handler)  {
    mInstrument=instrument;
    mMidi=handler;
  }

  void Process() {
    mMidi->Listen();
    if (mMidi->HasEvents()) {
      daisy::MidiEvent msg=mMidi->PopEvent();
      DispatchEvent(msg);
    }
  }
  
 private:
  daisy::MidiHandler<Transport> *mMidi;
};

#endif
