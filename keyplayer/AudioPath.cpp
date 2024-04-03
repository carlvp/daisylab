#include "keyplayer.h"
#include "Channel.h"

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
      out[0][i] = buffer[2*i];
      out[1][i] = buffer[2*i+1];
    }
    ++nBlocksConsumed;
  }
  /* else: buffer overrun */
}

void startAudioPath() {
  nBlocksProduced=2; /* two empty blocks */
  nBlocksConsumed=0;
  DaisySeedHw.SetAudioBlockSize(BLOCK_SIZE);
  DaisySeedHw.StartAudio(AudioCallback);
}

void processAudioPath() {
  if (nBlocksProduced-nBlocksConsumed < 2) {
    /* there is a free block: fill it */
    float *stereoOut=stereoOutDoubleBuffer[nBlocksProduced & 1];
    const float *result=Channel::mixAllChannels(stereoOut, zeroBuffer);

    // zero fill if there were no active channels
    if (result==zeroBuffer)
      memset(stereoOut, 0, sizeof(float)*2*BLOCK_SIZE);
    
    nBlocksProduced++;
  }
}

unsigned getAudioPathTimestamp() {
  return nBlocksProduced;
}
