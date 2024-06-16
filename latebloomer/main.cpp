/** Simple example of using USB MIDI 
 * 
 *  When the project boots up, a 100Hz sine wave will emit from both outputs,
 *  and the Daisy should appear as an Audio/MIDI device on a connected host.
 * 
 *  To keep the example short, only note on messages are handled, and there
 *  is only a single oscillator voice that tracks the most recent note message.
 */

#include "configuration.h"
#include "hardware.h"
#include "AudioPath.h"
#include "Instrument.h"
#include "MidiDispatcher.h"
#include "Program.h"

#ifdef CONFIG_DAISY_POD
#include <daisy_pod.h>
static daisy::DaisyPod DaisyHw;
#define DaisySeedHw DaisyHw.seed
#else
#include <daisy_seed.h>
static daisy::DaisySeed DaisyHw;
#define DaisySeedHw DaisyHw
#endif

Instrument theOp6Daisy;

static bool gateLED, underrunLED;

void setGateLED(bool ledState) {
  gateLED=ledState;
  // We just use a single LED
  DaisySeedHw.SetLed(gateLED | underrunLED);
}

void setUnderrunLED(bool ledState) {
  underrunLED=ledState;
  // We just use a single LED
  DaisySeedHw.SetLed(gateLED | underrunLED);
}

void startAudioCallback(daisy::AudioHandle::AudioCallback callback,
			unsigned blockSize) {
  DaisySeedHw.audio_handle.SetBlockSize(blockSize);
  DaisyHw.StartAudio(callback);
}

#ifdef CONFIG_USB_MIDI
static MidiDispatcher<daisy::MidiUsbTransport> theMidiDispatcher;

// variant 1: MIDI over USB
static daisy::MidiUsbHandler *createMidiHandler() {
  static daisy::MidiUsbHandler theMidiHandler;
  daisy::MidiUsbHandler::Config midi_cfg;
  
  midi_cfg.transport_config.periph = daisy::MidiUsbTransport::Config::INTERNAL;
  theMidiHandler.Init(midi_cfg);

  return &theMidiHandler;
}

#else
static MidiDispatcher<daisy::MidiUartTransport> theMidiDispatcher;

#ifdef CONFIG_DAISY_POD
// Variant 2: MIDI from "real MIDI port" on Daisy Pod (already initialized)
static daisy::MidiUartHandler *createMidiHandler() {
  // easy, since UART midi has already been set up
  return &DaisyHw.midi;
}
#else
// Variant 3: MIDI from "real MIDI port" on Daisy Seed (needs to create handler)
static daisy::MidiUartHandler *createMidiHandler() {
  static daisy::MidiUartHandler theMidiHandler;
  daisy::MidiUartHandler::Config midi_cfg;
  theMidiHandler.Init(midi_cfg);

  return &theMidiHandler;
}
#endif
#endif

int main(void)
{
  // Init
  DaisyHw.Init();
  theOp6Daisy.Init();
  auto pMidiHandler=createMidiHandler();
  theMidiDispatcher.Init(&theOp6Daisy, pMidiHandler);

  // Main loop
  DaisySeedHw.SetLed(false); // LED signals gate + buffer underrun
  pMidiHandler->StartReceive();
  startAudioPath();
  while(1) {
    processAudioPath(&theOp6Daisy);
    theMidiDispatcher.Process();
  }
}
