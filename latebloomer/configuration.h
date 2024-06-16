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
#define CONFIG_DAISY_SEED
//#define CONFIG_DAISY_POD

// MIDI over USB or MIDI-in from MIDI Port
// configure exactly one of them, comment out the other
#define CONFIG_USB_MIDI
//#define CONFIG_UART_MIDI

#endif
