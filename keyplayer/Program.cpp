#include "Program.h"

static const Program programBank[]={
  { // Program 0
    name: "Init Voice",
    algorithm: 1,
    op: {
      { // OP6
        fixedFreq:  false,
        freq:       1.00f,
        totalLevel: 0.00f,
        attack:     0.001f,
        decay:      0.001f,
        sustain:    1.00f,
        release:    0.001f
      },
      { // OP5
        fixedFreq:  false,
        freq:       1.00f,
        totalLevel: 0.00f,
        attack:     0.001f,
        decay:      0.001f,
        sustain:    1.00f,
        release:    0.001f
      },
      { // OP4
        fixedFreq:  false,
        freq:       1.00f,
        totalLevel: 0.00f,
        attack:     0.001f,
        decay:      0.001f,
        sustain:    1.00f,
        release:    0.001f
      },
      { // OP3
        fixedFreq:  false,
        freq:       1.00f,
        totalLevel: 0.00f,
        attack:     0.001f,
        decay:      0.001f,
        sustain:    1.00f,
        release:    0.001f
      },
      { // OP2
        fixedFreq:  false,
        freq:       1.00f,
        totalLevel: 0.00f,
        attack:     0.001f,
        decay:      0.001f,
        sustain:    1.00f,
        release:    0.001f
      },
      { // OP1
        fixedFreq:  false,
        freq:       1.00f,
        totalLevel: 1.00f,
        attack:     0.001f,
        decay:      0.001f,
        sustain:    1.00f,
        release:    0.001f
      },
    }
  },
  { // Program 17
    name: "E.Organ 1",
    algorithm: 32,
    op: {
      { // OP6
        fixedFreq:  false,
        freq:       3.00f,
        totalLevel: 0.10f,
        attack:     0.01f,
        decay:      0.08f,
        sustain:    0.00f,
        release:    0.01f
      },
      { // OP5
        fixedFreq:  false,
        freq:       1.006f,
        totalLevel: 0.10f,
        attack:     0.01f,
        decay:      1.00f,
        sustain:    1.00f,
        release:    0.01f
      },
      { // OP4
        fixedFreq:  false,
        freq:       0.508f,
        totalLevel: 0.10f,
        attack:     0.01f,
        decay:      1.00f,
        sustain:    1.00f,
        release:    0.01f
      },
      { // OP3
        fixedFreq:  false,
        freq:       1.520f,
        totalLevel: 0.10f,
        attack:     0.01f,
        decay:      1.00f,
        sustain:    1.00f,
        release:    0.04f
      },
      { // OP2
        fixedFreq:  false,
        freq:       0.990f,
        totalLevel: 0.10f,
        attack:     0.01f,
        decay:      1.00f,
        sustain:    1.00f,
        release:    0.01f
      },
      { // OP1
        fixedFreq:  false,
        freq:       0.497f,
        totalLevel: 0.10f,
        attack:     0.01f,
        decay:      1.00f,
        sustain:    1.00f,
        release:    0.01f
      }
    }
  }
};

const Program *Program::getProgram(unsigned programNumber) {
  if (programNumber>=sizeof(programBank)/sizeof(Program))
    programNumber=0;
  return &programBank[programNumber];
}
