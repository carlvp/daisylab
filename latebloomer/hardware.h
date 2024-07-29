#pragma once
#ifndef HARDWARE_H
#define HARDWARE_H

#include <daisy.h>
#include "configuration.h"

// set state of LEDs
// Currently they are multiplexed using a single LED ("on" if either is "on")
void setGateLED(bool ledState);
void setUnderrunLED(bool ledState);

// start the audio callback
void startAudioCallback(daisy::AudioHandle::AudioCallback callback,
			unsigned blockSize);

#ifdef WITH_FILTER_KNOBS
// get paramter value [0,16383]
// knobIndex=0 for POT1, knobIndex=1 for POT2
unsigned short getKnob(unsigned knobIndex);

#endif
#endif
