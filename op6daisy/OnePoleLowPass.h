#pragma once
#ifndef OnePoleLowpass_H
#define OnePoleLowpass_H

#include <cmath>
#include "configuration.h"

// One-pole, non-resonant lowpass filter
// H(z) = g/(1-az^(-1)), g=1-a for unity gain at DC

class OnePoleLowpass {
 public:
  OnePoleLowpass()
    : m_a{0.0f},
      mGain{1.0f},
      mDelay{0.0f}
  {
  }

  explicit OnePoleLowpass(float a)
    : m_a{a},
      mGain{1.0f-a},
      mDelay{0.0f}
  {
  }

  float getNextSample(float input) {
    return mDelay=input*mGain + m_a*mDelay;
  }

  void setCoefficient(float a) {
    m_a=a;
    mGain=1.0f-a;
  }

  // Coefficient for lowpass with (half-power) cutoff at hz
  static constexpr float hz2Coefficient(float hz) {
    float theta=2*M_PI*hz/SAMPLE_RATE;
    return rad2Coefficient(theta);
  }

  // Coefficient for lowpass with (half-power) cutoff at given rad/sample
  static constexpr float rad2Coefficient(float theta) {
    float K=2-std::cos(theta);
    return K-sqrtf(K*K-1);
  }

private:
  float m_a, mGain, mDelay;
};

#endif
