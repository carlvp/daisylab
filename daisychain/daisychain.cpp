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

static daisy::DaisySeed    seed;
static daisy::Switch       sw1;
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

static void AudioCallback(daisy::AudioHandle::InputBuffer  in,
			  daisy::AudioHandle::OutputBuffer out,
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
      out[0][i] = out[1][i] = nextBlock[i];
    }
    numBlocksConsumed++;
  }
  else {
    // Here the buffer is underrun, turn the LED on to indicate error
    seed.SetLed(true);
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
  seed.Init();

  // Initialize button (D15)
  sw1.Init(daisy::seed::D15, 1000);

  // Audio configuration
  seed.SetAudioBlockSize(BLOCK_SIZE);
  seed.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);

  // ...and the synth components
  float samplerate=seed.AudioSampleRate();
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
  seed.StartAudio(AudioCallback);

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
