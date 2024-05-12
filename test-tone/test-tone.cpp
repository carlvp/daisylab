// This program produces a 440 Hz test signal on
// Audio Out 1 and 2 (left/right stereo channels)
//
// Relevant pins on the Daisy Seed board:
// 18 Audio Out 1
// 19 Audio Out 2
// 20 Analog Ground
// 40 Digital Ground (Analog and Digital Ground should be connected)

#include <math.h>
#include <daisy_seed.h>

// Interface to the Daisy Seed hardware
static daisy::DaisySeed      hw;

static constexpr float amplitude=0.5f;
static constexpr float samplingFreq=48000.0f;
static constexpr float toneFreqHz=440.0f;

// per-sample phase increment (radians)
static constexpr float deltaPhi=2*M_PI*toneFreqHz/samplingFreq;

// 1 ms worth of samples @ 48 kHz
static constexpr unsigned BLOCK_SIZE=48;

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
			  size_t                    size)
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
    // >Here the buffer is underrun, turn the LED on to indicate error
    hw.SetLed(true);
  }
}

static void fillBuffer(float *buffer, size_t size, float phi0) {
  for(unsigned i=0; i<size; i++) {
    buffer[i] = amplitude*sinf(phi0+deltaPhi*i);
  }
}

int main(void)
{
  float phi=0;
  
  // Initialization of Daisy hardware
  hw.Init();
  
  // the buffers are initially zero
  // give the producer two blocks head start to get things going
  // this inserts 2ms of silence at the start of the stream
  numBlocksProduced=2;
  numBlocksConsumed=0;
    
  // Set block size, sample rate and start the audio callback
  hw.SetAudioBlockSize(BLOCK_SIZE);
  hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);
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
      fillBuffer(nextBlock, BLOCK_SIZE, phi);
      phi+=deltaPhi*BLOCK_SIZE;
      // This is just to prevent phi to grow out of bounds (precision loss)
      if (phi>(float) M_PI)
	phi-=(2*M_PI);
      numBlocksProduced++;
    }
  }
}
