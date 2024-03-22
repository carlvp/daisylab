#include "keyplayer.h"
#include "MidiDispatcher.h"
#include "Voice.h"

using namespace daisy;

Program theProgram = {
 name: "E.Organ 1",
 op:
  {
    { // OP6
      fixedFreq:  false,
      freq:       3.00f,
      totalLevel: 0.10f,
      attack:     0.01f,
      decay:      0.08f,
      sustain:    0.00f,
      release:    0.01f
    },
    { // OP5
      fixedFreq:  false,
      freq:       1.006f,
      totalLevel: 0.10f,
      attack:     0.01f,
      decay:      1.00f,
      sustain:    1.00f,
      release:    0.01f
    },
    { // OP4
      fixedFreq:  false,
      freq:       0.508f,
      totalLevel: 0.10f,
      attack:     0.01f,
      decay:      1.00f,
      sustain:    1.00f,
      release:    0.01f
    },    
    { // OP3
      fixedFreq:  false,
      freq:       1.520f,
      totalLevel: 0.10f,
      attack:     0.01f,
      decay:      1.00f,
      sustain:    1.00f,
      release:    0.04f
    },
    { // OP2
      fixedFreq:  false,
      freq:       0.990f,
      totalLevel: 0.10f,
      attack:     0.01f,
      decay:      1.00f,
      sustain:    1.00f,
      release:    0.01f
    },
    { // OP1
      fixedFreq:  false,
      freq:       0.497f,
      totalLevel: 0.10f,
      attack:     0.01f,
      decay:      1.00f,
      sustain:    1.00f,
      release:    0.01f
    }
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
