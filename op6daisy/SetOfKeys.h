#pragma once
#ifndef SetOfKeys_H
#define SetOfKeys_H

#include <cstdint>

// A special flavour of a bitset, which has min() and max()
// Represents sets of MIDI keys

class SetOfKeys {
public:
  SetOfKeys() {
    clearAll();
  }

  bool get(unsigned k) const {
    return (mBits[k/32] & mask(k)) != 0;
  }

  void set(unsigned k) {
    if (k<128) mBits[k/32] |= mask(k);
  }

  void reset(unsigned k) {
    if (k<128) mBits[k/32] &= ~mask(k);
  }

  void clearAll() {
    mBits[3] = mBits[2] = mBits[1] = mBits[0] = 0;
  }

  bool any() const {
    return (mBits[0] | mBits[1] | mBits[2] | mBits[3])!=0;
  }

  bool none() const {
    return !any();
  }

  int min() const {
    unsigned i;
    for (i=0; i<4; ++i)
      if (mBits[i]) {
	uint32_t bits=mBits[i];
	unsigned b=32*i;
	if ((bits & 0xffff)==0) { b+=16; bits>>=16; }
	if ((bits & 0xff)==0)   { b+=8; bits>>=8; }
	// now we know for sure that one of b0..b7 is set
	for (; (bits & 1)==0; bits>>=1)
	  b++;
	return b;
      }
    return -1;
  }

  unsigned max() const {
    unsigned i=4;
    while (i--)
      if (mBits[i]) {
	uint32_t bits=mBits[i];
	unsigned b=32*i;
	if ((bits & 0xffff0000)!=0) { b+=16; bits>>=16; }
	if ((bits & 0xff00)!=0)     { b+=8;  bits>>=8; }
	// now we know for sure that one of b0..b7 is set
	for (b+=7; (bits & 0x80)==0; bits<<=1)
	  b--;
	return b;
      }
    return -1;
  }

private:
  uint32_t mBits[4];

  uint32_t mask(unsigned k) const {
    return (1 << (k & 31));
  }
};

#endif
