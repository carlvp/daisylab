#include "Program.h"

#define INIT_OP(TL) { \
  fixedFreq:  false,  \
  freq:       1.000,  \
  totalLevel: TL,     \
  envelope: {         \
    level0:  0.000000,\
    levels: {1.000000, 1.000000, 1.000000, 0.000000}, \
    times:  {0.001500, 0.001500, 0.001500, 0.001500}, \
  } \
}
    
static const Program programBank[]={
  {
    name: "INIT VOICE",
    algorithm: 1,
    op: {
      INIT_OP(0),
      INIT_OP(0),
      INIT_OP(0),
      INIT_OP(0),
      INIT_OP(0),
      INIT_OP(1.0),
    }
  },
#include "EOrgan1.i"
};

const Program *Program::getProgram(unsigned programNumber) {
  if (programNumber>=sizeof(programBank)/sizeof(Program))
    programNumber=0;
  return &programBank[programNumber];
}
