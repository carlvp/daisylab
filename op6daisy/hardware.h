#pragma once
#ifndef HARDWARE_H
#define HARDWARE_H

#include <daisy.h>

// set state of LEDs
// Currently they are multiplexed using a single LED ("on" if either is "on")
void setGateLED(bool ledState);
void setUnderrunLED(bool ledState);
void setErrorLED(bool ledState);

// set LED on failed assert
#define assert_with_led(COND) { if (!(COND)) setErrorLED(true); }

// start the audio callback
void startAudioCallback(daisy::AudioHandle::AudioCallback callback,
			unsigned blockSize);

#endif
