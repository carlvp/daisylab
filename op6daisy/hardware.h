#pragma once
#ifndef HARDWARE_H
#define HARDWARE_H

#include <daisy_seed.h>

extern daisy::DaisySeed DaisySeedHw;

// set state of LEDs
// Currently they are multiplexed using a single LED ("on" if either is "on")
void setGateLED(bool ledState);
void setUnderrunLED(bool ledState);

#endif
