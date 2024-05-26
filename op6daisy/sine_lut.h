#pragma once
#ifndef SINE_LUT_H
#define SINE_LUT_H

extern const int sine_lut_10b[1025];

static inline int sine_lut(int x) {
  static constexpr int SIGN_BIT  =0x80000000;
  static constexpr int MIRROR_BIT=0x40000000;
  static constexpr int numLutBits=10;
  int sign=(x & SIGN_BIT)>>31;  // 0 or -1
  
  if (x & MIRROR_BIT) x=~x;
  unsigned index=(x>>(32-2-numLutBits)) & ((1<<numLutBits)-1);
  unsigned short dx=(x>>(32-2-numLutBits-16));
  int y=sine_lut_10b[index];
  unsigned short dy=sine_lut_10b[index+1]-y;

  // Interpolate between sine_lut[index] and sine_lut[index+1] 
  y += (dx*dy)>>16;
  // Conditional change of sign
  return (y + sign) ^ sign;
}

#endif
