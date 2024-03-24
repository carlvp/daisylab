#pragma once
#ifndef Program_H
#define Program_H

#include "KeyplayerConfig.h"

struct FmOperatorParam {
  bool fixedFreq;
  float freq;              // Fixed frequency (Hz) or frequency ratio
  float totalLevel;
  float attack, decay, sustain, release;
};

struct Program {
  const char *name;
  unsigned char algorithm;
  FmOperatorParam op[NUM_OPERATORS];

  static const Program *getProgram(unsigned programNumber);
};

#endif
