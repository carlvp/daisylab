#include "keyplayer.h"
#include "MidiDispatcher.h"
#include "Voice.h"
#include "Program.h"

void UsbMidiDispatcher::Init() {
  daisy::MidiUsbHandler::Config midi_cfg;
  midi_cfg.transport_config.periph = daisy::MidiUsbTransport::Config::INTERNAL;
  mMidi.Init(midi_cfg);

  for (unsigned ch=0; ch<16; ++ch)
    mChannel[ch].program=Program::getProgram(1);
}

void UsbMidiDispatcher::noteOn(unsigned channel, unsigned key, unsigned velocity) {
  const Channel *ch=&mChannel[channel];
  Voice *voice=allocateVoice(channel, key);
  voice->noteOn(ch, key, velocity);
  DaisySeedHw.SetLed(true);
}

void UsbMidiDispatcher::noteOff(unsigned channel, unsigned key) {
  Voice *voice=findVoice(channel, key);
  if (voice) {
    voice->noteOff();
    DaisySeedHw.SetLed(false);
  }
}

void UsbMidiDispatcher::programChange(unsigned channel, unsigned program) {
  mChannel[channel].program=Program::getProgram(program);
}

void UsbMidiDispatcher::DispatchEvents() {
  while (mMidi.HasEvents()) {
    auto msg = mMidi.PopEvent();
    switch(msg.type) {
    case daisy::NoteOn:
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
    case daisy::NoteOff:
      noteOff(msg.channel, msg.AsNoteOff().note);
      break;
    case daisy::ProgramChange:
      programChange(msg.channel, msg.data[0]);
      break;
    default:
      break;
    }
  }
}
