#include "keyplayer.h"
#include "MidiDispatcher.h"
#include "Voice.h"

using namespace daisy;

Program theProgram = {
 name: "Example Program",
 op: {
      totalLevel: 0.71f,
      attack:     0.01f,
      decay:      0.40f,
      sustain:    0.25f,
      release:    0.10f
 }
};

static void noteOn(unsigned channel, unsigned key, unsigned velocity) {
  Voice *voice=allocateVoice(channel, key);
  voice->noteOn(&theProgram, key, velocity);
  DaisySeedHw.SetLed(true);
}

static void noteOff(unsigned channel, unsigned key) {
  Voice *voice=findVoice(channel, key);
  if (voice) {
    voice->noteOff();
    DaisySeedHw.SetLed(false);
  }
}

void UsbMidiDispatcher::DispatchEvents() {
  while (mMidi.HasEvents()) {
    auto msg = mMidi.PopEvent();
    switch(msg.type) {
    case NoteOn:
      {
	auto note_msg = msg.AsNoteOn();
	if(note_msg.velocity != 0) {
	  noteOn(note_msg.channel, note_msg.note, note_msg.velocity);  
	}
	else {
	  noteOff(note_msg.channel, note_msg.note);
	}
      }
      break;
    case NoteOff:
      noteOff(msg.channel, msg.AsNoteOff().note);
      break;
    default:
      break;
    }
  }
}
