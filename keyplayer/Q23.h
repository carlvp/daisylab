#pragma once
#ifndef Q23_H
#define Q23_H

#include <cstdint>

// Samples are Q23 (signed fixed point, 23 fractional bits)

namespace Q23 {
  // 1.0 is represented by Q23::UNITY
  static constexpr std::int32_t UNITY=0x00800000;

  // The range [-1.0, +1.0) which fits in 24 bits is from Q23_MIN
  // to Q23_MAX (inclusive).
  // When represented as int32_t (32 bits) there is headroom (8 guard bits).
  static constexpr std::int32_t MAX  =0x007fffff;
  static constexpr std::int32_t MIN  =0xff800000;

  constexpr std::int32_t fromFloat(float f) { return UNITY*f; }
  constexpr float toFloat(std::int32_t q) { return ((float) q)/UNITY; }
};

#endif
