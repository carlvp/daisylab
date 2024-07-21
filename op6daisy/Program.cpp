#include "configuration.h"
#include "Program.h"

// This is lfo speed 35 (SYX parameter) 5.794 Hz
static constexpr int initLfoSpeed=ldexpf(5.794f*BLOCK_SIZE/SAMPLE_RATE, 32);

LfoParam::LfoParam()
  : waveform{0}, delay{0}, deltaPhi{initLfoSpeed}
{
}

EnvelopeParam::EnvelopeParam(float l0)
  : level0{l0},
    levels{1.0f, 1.0f, 1.0f, l0},
    times{0.001500, 0.001500, 0.001500, 0.001500}
{
}

KeyScalingParam::KeyScalingParam()
  : bp{60}, lcExp{false}, lDepth{0}, rcExp{false}, rDepth{0}
{
}

FmOperatorParam::FmOperatorParam()
  : fixedFreq{false},
    velScaling{0},
    kbdRateScaling{0},
    ams{0},
    freq{1.0f},
    totalLevel{0.0f},
    envelope{0.0f}
{
}

Program::Program() 
  : algorithm{0},
    feedback{0},
    lfoPmDepth{0.0f},
    lfoAmDepth{0.0f},
    pitchEnvelope{1.0f}
{
  op[5].totalLevel=1.0f;
}
