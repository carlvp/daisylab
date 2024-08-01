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

#ifdef WITH_SAI2
// This function is adapted from daisy_patch.cpp
//
// In addition to the (usual) SAI1 interface, it sets up a second
// digital audio interface (SAI2).
// audioConfig     Configuration, which is used to set up both SAI1, which
//                 is the serial audio interface of the usual, on-board codec,
//                 and SAI2, which is available on pins 31-35. 
//                 Default constructor AudioHandle::Config() sets up
//                 blocksize=48,
//                 samplerate=SAI_48KHZ,
//                 postgain=1.0,
//                 output_compensation=1.0
// sai2ClockMaster=true makes SAI2 Audio Block B a clock master, which means
//                 that it generates clock signals on pins 31, 34 and 35.
//                 This is useful when driving an external codec.
//                =false makes it instead sync to the clocks received on pins
//                 34 and 35 (pin 31 unused in this case).
//                 Audio block A (serial data output on pin 33) is synced to
//                 the same clock as audio block B in both cases.

static void InitAudioWithSAI2(daisy::AudioHandle::Config audioConfig,
			      bool sai2ClockMaster) {
  // Handle Seed Audio as-is and then
  daisy::SaiHandle::Config sai_config[2];
  // Internal Codec
  if(DaisySeedHw.CheckBoardVersion() == daisy::DaisySeed::BoardVersion::DAISY_SEED_1_1)
  {
    sai_config[0].pin_config.sa = daisy::Pin(daisy::PORTE, 6);
    sai_config[0].pin_config.sb = daisy::Pin(daisy::PORTE, 3);
    sai_config[0].a_dir         = daisy::SaiHandle::Config::Direction::RECEIVE;
    sai_config[0].b_dir         = daisy::SaiHandle::Config::Direction::TRANSMIT;
  }
  else
  {
    sai_config[0].pin_config.sa = daisy::Pin(daisy::PORTE, 6);
    sai_config[0].pin_config.sb = daisy::Pin(daisy::PORTE, 3);
    sai_config[0].a_dir         = daisy::SaiHandle::Config::Direction::TRANSMIT;
    sai_config[0].b_dir         = daisy::SaiHandle::Config::Direction::RECEIVE;
  }
  sai_config[0].periph          = daisy::SaiHandle::Config::Peripheral::SAI_1;
  sai_config[0].sr              = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
  sai_config[0].bit_depth       = daisy::SaiHandle::Config::BitDepth::SAI_24BIT;
  sai_config[0].a_sync          = daisy::SaiHandle::Config::Sync::MASTER;
  sai_config[0].b_sync          = daisy::SaiHandle::Config::Sync::SLAVE;
  sai_config[0].pin_config.fs   = daisy::Pin(daisy::PORTE, 4);
  sai_config[0].pin_config.mclk = daisy::Pin(daisy::PORTE, 2);
  sai_config[0].pin_config.sck  = daisy::Pin(daisy::PORTE, 5);

  // External Codec
  sai_config[1].periph          = daisy::SaiHandle::Config::Peripheral::SAI_2;
  sai_config[1].sr              = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
  sai_config[1].bit_depth       = daisy::SaiHandle::Config::BitDepth::SAI_24BIT;
  // There is no use making Audio Block A a master, since none of Daisy Seed's
  // pins has those functions (FS, SCK, MCLK).
  sai_config[1].a_sync          = daisy::SaiHandle::Config::Sync::SLAVE;
  // but Audio Block B *can* be MASTER
  sai_config[1].b_sync          = (sai2ClockMaster)?
    daisy::SaiHandle::Config::Sync::MASTER : daisy::SaiHandle::Config::Sync::SLAVE;
  // if we want to use the AudioCallback, I think we need to keep these as is,
  // but in theory we could have two stereo outputs or two stereo inputs.
  sai_config[1].a_dir           = daisy::SaiHandle::Config::Direction::TRANSMIT;
  sai_config[1].b_dir           = daisy::SaiHandle::Config::Direction::RECEIVE;
  sai_config[1].pin_config.fs   = daisy::seed::D27;
  sai_config[1].pin_config.mclk = daisy::seed::D24;
  sai_config[1].pin_config.sck  = daisy::seed::D28;
  sai_config[1].pin_config.sb   = daisy::seed::D25;
  sai_config[1].pin_config.sa   = daisy::seed::D26;

  daisy::SaiHandle sai_handle[2];
  sai_handle[0].Init(sai_config[0]);
  sai_handle[1].Init(sai_config[1]);

  // Reinit Audio for _both_ codecs...
  DaisySeedHw.audio_handle.Init(audioConfig, sai_handle[0], sai_handle[1]);

  // When SAI2 Audio Block B is set up to receive clock signal, it should be
  // in asynchronous mode (SCK and FS are inputs in this case).
  // SaiHandle::Init assumes it is synchronous, so we have to fix it here.
  if (!(sai2ClockMaster)) {
    // SYNCEN[1:0]=00 means ASYNC (master/slave)
    SAI2_Block_B->CR1 &= ~SAI_xCR1_SYNCEN_Msk;
  }
}

static void initAudioPath() {
  bool generateClock=true;
  daisy::AudioHandle::Config audioConfig;
  
  // Configure SAI1 and SAI2
  audioConfig.blocksize=BLOCK_SIZE;
  InitAudioWithSAI2(audioConfig, generateClock);
}

#else // without SAI2

static void initAudioPath() {
}

#endif

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
  initAudioPath();
  
  // Main loop
  DaisySeedHw.SetLed(false); // LED signals gate + buffer underrun
  pMidiHandler->StartReceive();
  startAudioPath();
  while(1) {
    processAudioPath(&theOp6Daisy);
    theMidiDispatcher.Process();
  }
}
