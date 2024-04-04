#include "hardware.h"
#include "Channel.h"
#include "Instrument.h"
#include "MidiDispatcher.h"
#include "Voice.h"

void UsbMidiDispatcher::Init(Instrument *instr) {
  daisy::MidiUsbHandler::Config midi_cfg;
  midi_cfg.transport_config.periph = daisy::MidiUsbTransport::Config::INTERNAL;
  mMidi.Init(midi_cfg);
  mInstrument=instr;
}

void UsbMidiDispatcher::DispatchEvents() {
  while (mMidi.HasEvents()) {
    auto msg = mMidi.PopEvent();
    switch(msg.type) {
    case daisy::NoteOn:
      {
	auto note_msg = msg.AsNoteOn();
	if(note_msg.velocity != 0) {
	  mInstrument->noteOn(note_msg.channel,
			      note_msg.note,
			      note_msg.velocity);
	  DaisySeedHw.SetLed(true);
	}
	else {
	  mInstrument->noteOff(note_msg.channel,
			       note_msg.note);
	  DaisySeedHw.SetLed(false);
	}
      }
      break;
    case daisy::NoteOff:
      mInstrument->noteOff(msg.channel, msg.AsNoteOff().note);
      DaisySeedHw.SetLed(false);
      break;
    case daisy::ControlChange:
      mInstrument->controlChange(msg.channel, msg.data[0], msg.data[1]);
      break;
    case daisy::ProgramChange:
      mInstrument->programChange(msg.channel, msg.data[0]);
      break;
    default:
      break;
    }
  }
}
