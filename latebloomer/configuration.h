#pragma once
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define SAMPLE_RATE  48000
#define BLOCK_SIZE      32
#define NUM_PROGRAMS    32
#define NUM_CHANNELS    16
#define NUM_VOICES      16
#define NUM_OPERATORS    4
#define NUM_ENV_STAGES   4
#define RELEASE_STAGE    3

// select hardware platform
//#define CONFIG_DAISY_SEED
#define CONFIG_DAISY_POD

#if defined(CONFIG_DAISY_SEED) && defined(CONFIG_DAISY_POD)
#error Configure ONE hardware platform
#endif

// MIDI over USB or MIDI-in from MIDI Port
// configure exactly one of them, comment out the other
#define CONFIG_USB_MIDI
//#define CONFIG_UART_MIDI

// Manual filter control, POT1=Cutoff frequency, POT2=Resonance
// Works out-of-the-box with daisy POD, use pin 28 (A6) and pin 22 (A0)
// on a Daisy Seed
//#define WITH_FILTER_KNOBS

// Use SAI2 for digital audio output (in addition to analog audio output)
#define WITH_SAI2

#endif
