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
  LfoParam();
  
  unsigned char waveform;
  unsigned short delay;
  int deltaPhi;
};
  
struct EnvelopeParam {
  EnvelopeParam(float l0);
  
  float level0;
  float levels[NUM_ENV_STAGES]; 
  float times[NUM_ENV_STAGES]; 
};

struct KeyScalingCurve {
  KeyScalingCurve();

  float keyScaling(unsigned key) const;

  enum Type {
     kMinusLin,
     kMinusExp,
     kPlusExp,
     kPlusLin
  };
  char curve;
  char depth;
};

struct KeyScalingParam {
  KeyScalingParam();

  float keyScaling(unsigned key) const {
    return (key<bp)? left.keyScaling(bp-key) : right.keyScaling(key-bp);
  }

  unsigned char bp;
  KeyScalingCurve left;
  KeyScalingCurve right;
};

struct FmOperatorParam {
  FmOperatorParam();
  
  bool fixedFreq;
  unsigned char velScaling;
  unsigned char kbdRateScaling;
  unsigned char ams;
  float freq;              // Fixed frequency (Hz) or frequency ratio
  float totalLevel;
  EnvelopeParam envelope;
  KeyScalingParam keyScaling;
};

struct Program {
  Program();
  
  unsigned char algorithm;
  unsigned char feedback;
  unsigned char lfoPmInitDepth; // 0..99
  float lfoPmSensitivity;
  float lfoAmDepth;
  LfoParam lfo;
  EnvelopeParam pitchEnvelope;
  FmOperatorParam op[NUM_OPERATORS];
};

#endif
