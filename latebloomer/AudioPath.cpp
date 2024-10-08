#include <daisy_seed.h>
#include <per/sai.h>
#include <stm32h750xx.h>

#include "hardware.h"
#include "AudioPath.h"
#include "Instrument.h"

static int nBlocksProduced, nBlocksConsumed;

const float zeroBuffer[2*BLOCK_SIZE]={0};
static float stereoOutDoubleBuffer[2][2*BLOCK_SIZE];

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
			  daisy::AudioHandle::OutputBuffer out,
			  size_t size)
{
  if (nBlocksProduced-nBlocksConsumed > 0) {
    const float *buffer=stereoOutDoubleBuffer[nBlocksConsumed & 1];
  
    for(size_t i = 0; i < size; i++) {
#ifdef WITH_SAI2
      out[0][i] = out[2][i] = buffer[2*i];
      out[1][i] = out[3][i] = buffer[2*i+1];
#else
      out[0][i] = buffer[2*i];
      out[1][i] = buffer[2*i+1];
#endif      
    }
    ++nBlocksConsumed;
  }
  else // else: buffer underrun, turn the LED on
    setUnderrunLED(true);
}

void startAudioPath() {
  nBlocksProduced=2; /* two empty blocks */
  nBlocksConsumed=0;
  startAudioCallback(AudioCallback, BLOCK_SIZE);
}

void processAudioPath(Instrument *instrument) {
  if (nBlocksProduced-nBlocksConsumed < 2) {
    /* there is a free block: fill it */
    float *stereoOut=stereoOutDoubleBuffer[nBlocksProduced & 1];

    instrument->fillBuffer(stereoOut);
    nBlocksProduced++;
  }
}
