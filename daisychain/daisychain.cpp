// This program plays an A4 (440 Hz) note when a switch connected to pin 22 is
// pressed. It also transmits and receives digital audio on SAI2 (pins 31-35).
// The input is passed to a delay line and, mixed with the test signal, it is
// sent to the digital output.
//
// Relevant pins on the Daisy Seed board:
// 22 D15, connected to ground via a switch, "sw1".
// 31 SAI2 Block B MCLK
// 32 SAI2 SD B digital audio in (connected to microphone/ADC or digital audio
//    out of other daisy/Rapberry PI)
// 33 SAI2 SD A digital audio out (connected to external DAC or digital audio
//    in of other daisy/Raspberry PI)
// 34 SAI2 FS frame select/left-right clock
// 35 SAI2 SCK bit clock
// 38 3V3 Digital Voltage
// 40 Digital Ground

#include <math.h>
#include <daisy_seed.h>
#include <daisysp.h>
#include <per/sai.h>
#include <stm32h750xx.h>

using namespace daisy;

static DaisySeed    hw;

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
  // There is no use making Audio Block A a master, since none of Daisy Seed's
  // pins has those functions (FS, SCK, MCLK).
  sai_config[1].a_sync          = SaiHandle::Config::Sync::SLAVE;
  // but Audio Block B *can* be MASTER
  sai_config[1].b_sync          = (sai2ClockMaster)?
    SaiHandle::Config::Sync::MASTER : SaiHandle::Config::Sync::SLAVE;
  // if we want to use the AudioCallback, I think we need to keep these as is,
  // but in theory we could have two stereo outputs or two stereo inputs.
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

  // When SAI2 Audio Block B is set up to receive clock signal, it should be
  // in asynchronous mode (SCK and FS are inputs in this case).
  // SaiHandle::Init assumes it is synchronous, so we have to fix it here.
  if (!(sai2ClockMaster)) {
    // SYNCEN[1:0]=00 means ASYNC (master/slave)
    SAI2_Block_B->CR1 &= ~SAI_xCR1_SYNCEN_Msk;
  }
}

static Switch              sw1;
static daisysp::Oscillator osc;
static daisysp::AdEnv      eg;
static bool                gate;
static constexpr float initialAmp=0.25f;
static constexpr float toneFreqHz=440.0f;
static constexpr unsigned BLOCK_SIZE=48;  // 1 ms worth of samples @ 48 kHz
static constexpr unsigned delayInBlocks=378; // 378 ms

// double buffer: a second buffer is filled
// while the AudioCallback drains a first one
static float doubleBuffer[2][BLOCK_SIZE];
static float delayLine[delayInBlocks][BLOCK_SIZE];
const static float *delayLineEnd=delayLine[delayInBlocks];

// simple synchronization mechanism: just keep track of number
// of blocks produced and consumed.
// "volatile" to prevent compiler from making wrong assumptions 
// (the variables are accessed in interrupt context, "at any time").
static volatile unsigned numBlocksProduced, numBlocksConsumed;

static void AudioCallback(AudioHandle::InputBuffer  in,
			  AudioHandle::OutputBuffer out,
			  size_t size)
{
  static float *writePtr=delayLine[0];

  if (writePtr==delayLineEnd)
    writePtr=delayLine[0];
  
  // number of ready blocks will always be 0, 1 or 2
  // -even when blocks counters wrap around (after about 50 days of streaming)
  unsigned numBlocksReady=numBlocksProduced-numBlocksConsumed;
  if (numBlocksReady != 0) {
    // Here at least one block is ready, doubleBuffer[0] or [1]
    const float *readPtr = doubleBuffer[numBlocksConsumed & 1];
    
    for(unsigned i=0; i<size; i++) {
      // copy to both left and right stereo channels
      out[0][i] = out[1][i] = out[2][i] = out[3][i] = *readPtr++;
      *writePtr++ = 0.5*(in[2][i] + in[3][i]);
    }
    numBlocksConsumed++;
  }
}

static const float *fillBuffer(float *buffer, const float *delay, size_t size) {
  // If the button is pressed, trigger the envelope
  sw1.Debounce();
  bool b=sw1.Pressed();

  if (b!=gate) {
    gate=b;
    if (gate) eg.Trigger();
  }

  if (delay==delayLineEnd)
    delay=delayLine[0];
  
  for(unsigned i=0; i<size; i++) {
    float gain=initialAmp*eg.Process();
    float y=0.3*(*delay++);
    buffer[i] = gain*osc.Process() + y;
  }

  return delay;
}

int main(void)
{
  const float *delayLineReadPtr;
  
  // Initialization of Daisy hardware
  hw.Init();
  
  // Initialize button (D15) 
  // if D15 is grounded when starting up, this devices won't generate the clocks
  sw1.Init(seed::D15, 1000);
  // practice has shown that incorrect (LOW) readouts are avoided by a delay
  hw.DelayMs(1);
  gate=sw1.RawState(); 
  bool generateClock=!gate;

  hw.SetLed(generateClock);
  
  // Configure SAI2
  AudioHandle::Config audioConfig;
  audioConfig.blocksize=BLOCK_SIZE;
  InitAudioWithSAI2(audioConfig, generateClock);
  
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
  delayLineReadPtr=delayLine[2];

  // Start the audio callback
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
      delayLineReadPtr=fillBuffer(nextBlock, delayLineReadPtr, BLOCK_SIZE);
      numBlocksProduced++;
     }
  }
}
