#pragma once
#ifndef KEYPLAYER_H

#include <daisy_seed.h>

extern daisy::DaisySeed DaisySeedHw;

#define BLOCK_SIZE 32

void initVoices(float sampleRate);
void startAudioPath();
void processAudioPath();
unsigned getAudioPathTimestamp();

class Voice;
Voice *findVoice(unsigned channel, unsigned key);
Voice *allocateVoice(unsigned channel, unsigned key);

#endif
