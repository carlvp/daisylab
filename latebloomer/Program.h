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
  KeyScalingParam();

  unsigned char bp;
  bool lcExp;
  signed char lDepth;
  bool rcExp;
  signed char rDepth;
};

struct FmOperatorParam {
  bool fixedFreq;
  bool waveform5;          // Waveform #5 of TX81z
  unsigned char velScaling;
  unsigned char kbdRateScaling;
  unsigned char ams;
  float freq;              // Fixed frequency (Hz) or frequency ratio
  float totalLevel;
  EnvelopeParam envelope;
  KeyScalingParam keyScaling;
};

struct SyxVoiceParam;
struct SyxBulkFormat;

struct Program {
  unsigned char algorithm;
  unsigned char feedback;
  float lfoPmDepth;
  float lfoAmDepth;
  LfoParam lfo;
  EnvelopeParam pitchEnvelope;
  FmOperatorParam op[NUM_OPERATORS];

  void load(const SyxVoiceParam &syxVoice);
};

extern Program lateBloomer;

#endif
