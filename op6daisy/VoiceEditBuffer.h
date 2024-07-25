#pragma once
#ifndef VoiceEditBuffer_H
#define VoiceEditBuffer_H

#include "Program.h"

struct SyxVoiceParam;

class VoiceEditBuffer {
 public:
  VoiceEditBuffer();
  
  // set to initial program
  void loadInitialProgram();

  // set to given program
  void loadProgram(const Program &program);

  // import .syx Program
  void loadSyx(const SyxVoiceParam &syxVoice);

  // copy this buffer to given Program
  void storeProgram(Program &program) const;

  // set a specific program parameter
  // page:  Op6 (0), Op5 (1), ..., Op1 (5), Common voice parameters (6)
  // param: Parameter number
  // value: Parameter value, 14-bit range 0..3fff
  void setParameter(unsigned page, unsigned param, unsigned value);

 private:
  Program mProgram;
  unsigned char mOpEnabled;

  void setOperatorEnable(unsigned page, bool isEnabled);
};

#endif

