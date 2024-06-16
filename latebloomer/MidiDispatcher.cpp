#include "hardware.h"
#include "Channel.h"
#include "Instrument.h"
#include "MidiDispatcher.h"
#include "Voice.h"

void BasicMidiDispatcher::DispatchEvent(daisy::MidiEvent &msg) {
  switch(msg.type) {
  case daisy::NoteOn:
    {
      auto note_msg = msg.AsNoteOn();
      if(note_msg.velocity != 0) {
	mInstrument->noteOn(note_msg.channel,
			    note_msg.note,
			    note_msg.velocity);
	setGateLED(true);
      }
      else {
	mInstrument->noteOff(note_msg.channel,
			     note_msg.note);
	setGateLED(false);
      }
    }
    break;
  case daisy::NoteOff:
    mInstrument->noteOff(msg.channel, msg.AsNoteOff().note);
    setGateLED(false);
    break;
  case daisy::ControlChange:
    mInstrument->controlChange(msg.channel, msg.data[0], msg.data[1]);
    break;
  case daisy::ProgramChange:
    mInstrument->programChange(msg.channel, msg.data[0]);
    break;
  case daisy::PitchBend:
    mInstrument->pitchBend(msg.channel,
			   (int) (msg.data[1]*128 + msg.data[0]) - 8192);
    break;
  case daisy::SystemCommon:
    if (msg.sc_type==daisy::SystemExclusive) {
      mInstrument->sysEx(msg.sysex_data, msg.sysex_message_len);
    }
    break;
  default:
    break;
  }
}
