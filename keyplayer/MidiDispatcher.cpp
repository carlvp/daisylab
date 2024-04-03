#include "keyplayer.h"
#include "Channel.h"
#include "MidiDispatcher.h"
#include "Voice.h"

void UsbMidiDispatcher::Init() {
  daisy::MidiUsbHandler::Config midi_cfg;
  midi_cfg.transport_config.periph = daisy::MidiUsbTransport::Config::INTERNAL;
  mMidi.Init(midi_cfg);

  for (unsigned ch=0; ch<16; ++ch) {
    Channel::allChannels[ch].reset();
    Channel::allChannels[ch].setMasterVolume(0.5f);
  }
}

void UsbMidiDispatcher::noteOn(unsigned channel, unsigned key, unsigned velocity) {
  Channel *ch=&Channel::allChannels[channel];
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

void UsbMidiDispatcher::controlChange(unsigned channel, unsigned cc, unsigned value) {
  if (cc==7)
    Channel::allChannels[channel].setChannelVolume(value*128);
  else if (cc==10)
    Channel::allChannels[channel].setPan(value*128);
}

void UsbMidiDispatcher::programChange(unsigned channel, unsigned program) {
  Channel::allChannels[channel].setProgram(program);
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
    case daisy::ControlChange:
      controlChange(msg.channel, msg.data[0], msg.data[1]);
      break;
    case daisy::ProgramChange:
      programChange(msg.channel, msg.data[0]);
      break;
    default:
      break;
    }
  }
}
