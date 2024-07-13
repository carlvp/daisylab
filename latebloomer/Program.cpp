#include <cmath>
#include "configuration.h"
#include "Program.h"
#include "SyxBulkFormat.h"

// This is lfo speed 35 (SYX parameter) 5.794 Hz
static constexpr int initLfoSpeed=ldexpf(5.794f*BLOCK_SIZE/SAMPLE_RATE, 32);

KeyScalingParam::KeyScalingParam()
  : bp{60}, lcExp{false}, lDepth{0}, rcExp{false}, rDepth{0}
{
}

Program lateBloomer = {
 algorithm:  2,         // 4-op Algorithm #2 of TX81z (and DX11/21/27/100)
 feedback:   9,         // a.k.a. FB=7
 lfoPmDepth: 0.011772f, // PMD=7, PMS=5
 lfoAmDepth: 0,
 lfo: {
   waveform: LfoWaveform::WAVE_TRIANGLE,
   delay:    0,
   deltaPhi: 0x00b404c7 // 4.12 Hz
 },
 pitchEnvelope: {
   level0: 1.0f,
   levels: {1.0f, 1.0f, 1.0f, 1.0f},
   times:  {0.0015f, 0.001500f, 0.001500f, 0.001500f}
 },
 op: {
   // op[0] ~ OP4 on TX81z
   {
     fixedFreq: false,
     waveform5: false,
     velScaling: 3,
     kbdRateScaling: 0,
     ams: 0,
     freq: 1.0f,
     totalLevel: 0.167f, // Level 79   
     envelope: {
       level0: 0.0f,
       levels: {1.0f,   0.00852f,  0.0f, 0.0f},
       times:  {0.013f, 0.371892f, 0.037189f, 0.012802f}
     }
   },
   // op[1] ~ OP3 on TX81z
   {
     fixedFreq: false,
     waveform5: true,   // <-- The secret sauce
     velScaling: 1,
     kbdRateScaling: 1,
     ams: 0,
     freq: 0.9902285f,   // -17 cents detune
     totalLevel: 0.113f, // Level 71   
     envelope: {
       level0: 0.0f,
       levels: {1.0f,   0.0f,      0.0f,     0.0f},
       times:  {0.013f, 0.173369f, 0.01733f, 0.012802f}
     }
   },
   // op[2] ~ OP2 on TX81z
   {
     fixedFreq: false,
     waveform5: false,
     velScaling: 1,
     kbdRateScaling: 1,
     ams: 0,
     freq: 0.5f,
     totalLevel: 0.145f, // level 74
     envelope: {
       level0: 0.0f,
       levels: {1.0f,   0.0f,     0.0f,   0.0f},
       times:  {0.013f, 2.54368f, 0.250f, 0.012802f}
     }
   },
   // op[3] ~ OP1 on TX81z
   {
     fixedFreq: false,
     waveform5: false,
     velScaling: 0,
     kbdRateScaling: 1,
     ams: 0,
     freq: 0.5f,
     totalLevel: 0.7f,   
     envelope: {
       level0: 0.022f,
       levels: {1.0f,   1.0f,  0.440f, 0.0f},
       times:  {0.011f, 0.02f, 0.197f, 0.073f}
     }
   },
 }
};
