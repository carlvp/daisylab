#pragma once
#ifndef Program_H
#define Program_H

#include "KeyplayerConfig.h"

struct EnvelopeParam {
  float level0;
  float levels[NUM_ENV_STAGES]; 
  float times[NUM_ENV_STAGES]; 
};

struct FmOperatorParam {
  bool fixedFreq;
  float freq;              // Fixed frequency (Hz) or frequency ratio
  float totalLevel;
  EnvelopeParam envelope;
};

struct Program {
  const char *name;
  unsigned char algorithm;
  FmOperatorParam op[NUM_OPERATORS];

  static const Program *getProgram(unsigned programNumber);
};

#endif
