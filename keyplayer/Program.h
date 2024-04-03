#pragma once
#ifndef Program_H
#define Program_H

#include "configuration.h"

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
  FmOperatorParam op[NUM_OPERATORS];

  static const Program *getProgram(unsigned programNumber);
};

#endif
