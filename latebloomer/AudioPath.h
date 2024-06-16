#pragma once
#ifndef AudioPath_H
#define AudioPath_H

extern const float zeroBuffer[];

void startAudioPath();

class Instrument;
void processAudioPath(Instrument *stereoSoundSource);

#endif
