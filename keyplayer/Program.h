#pragma once
#ifndef Program_H
#define Program_H

#include "configuration.h"

enum LfoWaveform {
  WAVE_TRIANGLE,
  WAVE_SAW_DOWN,
  WAVE_SAW_UP,
  WAVE_SINE,
  WAVE_SQUARE,
  WAVE_SAMPLE_HOLD,		  
};

struct LfoParam {
  unsigned char waveform;
  unsigned short delay;
  int deltaPhi;
};
  
struct EnvelopeParam {
  float level0;
  float levels[NUM_ENV_STAGES]; 
  float times[NUM_ENV_STAGES]; 
};

struct KeyScalingParam {
  unsigned char bp;
  bool lcExp;
  signed char lDepth;
  bool rcExp;
  signed char rDepth;
};

struct FmOperatorParam {
  bool fixedFreq;
  unsigned char velScaling;
  unsigned char kbdRateScaling;
  float freq;              // Fixed frequency (Hz) or frequency ratio
  float totalLevel;
  EnvelopeParam envelope;
  KeyScalingParam keyScaling;
};

struct Program {
  const char *name;
  unsigned char algorithm;
  unsigned char feedback;
  float lfoPmDepth;
  LfoParam lfo;
  FmOperatorParam op[NUM_OPERATORS];

  static const Program *getProgram(unsigned programNumber);
};

#endif
