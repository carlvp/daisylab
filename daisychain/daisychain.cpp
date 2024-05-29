// This program produces a 440 Hz test signal on
// Audio Out 1 and 2 (left/right stereo channels)
//
// Relevant pins on the Daisy Seed board:
// 18 Audio Out 1
// 19 Audio Out 2
// 20 Analog Ground
// 22 D15, connected to ground via a switch, "sw1".
// 40 Digital Ground (Analog and Digital Ground should be connected)

#include <math.h>
#include <daisy_seed.h>
#include <daisysp.h>
#include <per/sai.h>

using namespace daisy;

static DaisySeed    hw;

// This function is adapted from daisy_patch.cpp
//
// In addition to the (usual) SAI1 interface, it sets up a second
// digital audio interface (SAI2).
//
// sai2ClockMaster check databook how that is supposed to work,
// daisy_patch sets RX to master and TX to slave (weird).

static void InitAudioWithSAI2(AudioHandle::Config audioConfig,
			      bool sai2ClockMaster)
{
  // Handle Seed Audio as-is and then
  SaiHandle::Config sai_config[2];
  // Internal Codec
  if(hw.CheckBoardVersion() == DaisySeed::BoardVersion::DAISY_SEED_1_1)
  {
    sai_config[0].pin_config.sa = Pin(PORTE, 6);
    sai_config[0].pin_config.sb = Pin(PORTE, 3);
    sai_config[0].a_dir         = SaiHandle::Config::Direction::RECEIVE;
    sai_config[0].b_dir         = SaiHandle::Config::Direction::TRANSMIT;
  }
  else
  {
    sai_config[0].pin_config.sa = Pin(PORTE, 6);
    sai_config[0].pin_config.sb = Pin(PORTE, 3);
    sai_config[0].a_dir         = SaiHandle::Config::Direction::TRANSMIT;
    sai_config[0].b_dir         = SaiHandle::Config::Direction::RECEIVE;
  }
  sai_config[0].periph          = SaiHandle::Config::Peripheral::SAI_1;
  sai_config[0].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
  sai_config[0].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
  sai_config[0].a_sync          = SaiHandle::Config::Sync::MASTER;
  sai_config[0].b_sync          = SaiHandle::Config::Sync::SLAVE;
  sai_config[0].pin_config.fs   = Pin(PORTE, 4);
  sai_config[0].pin_config.mclk = Pin(PORTE, 2);
  sai_config[0].pin_config.sck  = Pin(PORTE, 5);

  // External Codec
  sai_config[1].periph          = SaiHandle::Config::Peripheral::SAI_2;
  sai_config[1].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
  sai_config[1].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
  sai_config[1].a_sync          = SaiHandle::Config::Sync::SLAVE;
  sai_config[1].b_sync          = SaiHandle::Config::Sync::MASTER;
  sai_config[1].a_dir           = SaiHandle::Config::Direction::TRANSMIT;
  sai_config[1].b_dir           = SaiHandle::Config::Direction::RECEIVE;
  sai_config[1].pin_config.fs   = seed::D27;
  sai_config[1].pin_config.mclk = seed::D24;
  sai_config[1].pin_config.sck  = seed::D28;
  sai_config[1].pin_config.sb   = seed::D25;
  sai_config[1].pin_config.sa   = seed::D26;

  SaiHandle sai_handle[2];
  sai_handle[0].Init(sai_config[0]);
  sai_handle[1].Init(sai_config[1]);

  // Reinit Audio for _both_ codecs...
  hw.audio_handle.Init(audioConfig, sai_handle[0], sai_handle[1]);
}

static Switch              sw1;
static daisysp::Oscillator osc;
static daisysp::AdEnv      eg;
static bool                gate;

static constexpr float toneFreqHz=440.0f;
static constexpr unsigned BLOCK_SIZE=48;  // 1 ms worth of samples @ 48 kHz

// double buffer: a second buffer is filled
// while the AudioCallback drains a first one
static float doubleBuffer[2][BLOCK_SIZE];

// simple synchronization mechanism: just keep track of number
// of blocks produced and consumed.
// "volatile" to prevent compiler from making wrong assumptions 
// (the variables are accessed in interrupt context, "at any time").
static volatile unsigned numBlocksProduced, numBlocksConsumed;

static void AudioCallback(AudioHandle::InputBuffer  in,
			  AudioHandle::OutputBuffer out,
			  size_t size)
{
  // number of ready blocks will always be 0, 1 or 2
  // -even when blocks counters wrap around (after about 50 days of streaming)
  unsigned numBlocksReady=numBlocksProduced-numBlocksConsumed;
  if (numBlocksReady != 0) {
    // Here at least one block is ready, doubleBuffer[0] or [1]
    float *nextBlock = doubleBuffer[numBlocksConsumed & 1];
    for(unsigned i=0; i<size; i++) {
      // copy to both left and right stereo channels
      out[0][i] = out[1][i] = out[3][i] = out[4][i] = nextBlock[i];
    }
    numBlocksConsumed++;
  }
  else {
    // Here the buffer is underrun, turn the LED on to indicate error
    hw.SetLed(true);
  }
}

static void fillBuffer(float *buffer, size_t size) {
  // If the button is pressed, trigger the envelope
  sw1.Debounce();
  bool b=sw1.Pressed();

  if (b!=gate) {
    gate=b;
    if (gate) eg.Trigger();
  }
  
  for(unsigned i=0; i<size; i++) {
    float gain=eg.Process();
    buffer[i] = gain*osc.Process();
  }
}

int main(void)
{
  // Initialization of Daisy hardware
  hw.Init();

  // Initialize button (D15)
  sw1.Init(seed::D15, 1000);

  // Configure SAI2
  // AudioHandle::Config defult constructor sets up
  //   blocksize=48,
  //   samplerate=SAI_48KHZ,
  //   postgain=1.0,
  //   output_compensation=1.0
  AudioHandle::Config audioConfig;
  audioConfig.blocksize=BLOCK_SIZE;
  InitAudioWithSAI2(audioConfig, true);
  
  // ...and the synth components
  float samplerate=hw.AudioSampleRate();
  osc.Init(samplerate);
  osc.SetFreq(toneFreqHz);
  osc.SetAmp(1.0f);
  osc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
  eg.Init(samplerate);
  eg.SetTime(daisysp::ADENV_SEG_ATTACK, 0.01f);
  eg.SetTime(daisysp::ADENV_SEG_DECAY,  0.4f);

  // the buffers are initially zero
  // give the producer two blocks head start to get things going
  // this inserts 2ms of silence at the start of the stream
  numBlocksProduced=2;
  numBlocksConsumed=0;
    
  // Set block size, sample rate and start the audio callback
  hw.StartAudio(AudioCallback);

  // Just busy wait for a buffer to be available,
  // which happens when the AudioCallback has consumed it
  while(1) {
    // number of buffered blocks will always be 0, 1 or 2
    // -even when blocks counters wrap around (after about 50 days of streaming)
    unsigned numBlocksBuffered=numBlocksProduced-numBlocksConsumed;
    if (numBlocksBuffered<2) {
      // Here at least one block is free, doubleBuffer[0] or [1]
      float *nextBlock=doubleBuffer[numBlocksProduced & 1];
      fillBuffer(nextBlock, BLOCK_SIZE);
      numBlocksProduced++;
    }
  }
}
