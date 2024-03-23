#include <cstdint>
#include "keyplayer.h"
#include "Voice.h"
#include "Q23.h"

static int nBlocksProduced, nBlocksConsumed;

static std::int32_t outBuffer[2][BLOCK_SIZE];

static void AudioCallback(daisy::AudioHandle::InputBuffer in,
			  daisy::AudioHandle::OutputBuffer out,
			  size_t size)
{
  if (nBlocksProduced-nBlocksConsumed > 0) {
    const std::int32_t *buffer=outBuffer[nBlocksConsumed & 1];
  
    for(size_t i = 0; i < size; i++) {
      out[0][i] = out[1][i] = Q23::toFloat(buffer[i]);
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
    std::int32_t *buffer=outBuffer[nBlocksProduced & 1];
    memset(buffer, 0, BLOCK_SIZE*sizeof(std::int32_t));
    for (unsigned i=0; i<NUM_VOICES; ++i)
      allVoices[i].addToBuffer(buffer);
    nBlocksProduced++;
  }
}

unsigned getAudioPathTimestamp() {
  return nBlocksProduced;
}
