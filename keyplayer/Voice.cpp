#include "keyplayer.h"
#include "Channel.h"
#include "FmAlgorithm.h"
#include "Voice.h"
#include "Program.h"

void Voice::noteOn(Channel *ch, unsigned key, unsigned velocity,
		   unsigned timestamp) {
  if (mChannel!=ch) {
    // Unregister with old channel
    if (mChannel)
      mChannel->removeVoice(this);
    
    // Register with new channel
    ch->addVoice(this);
    mChannel=ch;
  }
  mProgram=ch->getProgram();
  mAlgorithm=FmAlgorithm::getAlgorithm(mProgram->algorithm);
  mKey=key;
  mGate=true;
  mTimestamp=timestamp;
  
  std::int32_t deltaPhi=theKeyPlayer.midiToPhaseIncrement(key);
  float com=0.25f/mAlgorithm->getNumOutputs();
  for (unsigned i=0; i<NUM_OPERATORS; ++i) {
    float outputScaling=mAlgorithm->isOutput(i)? com : 1.0f;
    mOp[i].noteOn(&mProgram->op[i], key, velocity, deltaPhi, outputScaling);
  }
}

void Voice::noteOff(unsigned timestamp) {
  for (unsigned i=0; i<NUM_OPERATORS; ++i)
    mOp[i].noteOff(&mProgram->op[i]);
  mGate=false;
  mTimestamp=timestamp;
}

void Voice::fillBuffer(float *monoOut, const float *monoIn) {
  float pitchMod=mChannel->getPitchBendFactor();
  float lfo=1.0f;
  
  if (mAlgorithm) {
    mAlgorithm->fillBuffer(monoOut, monoIn, mOp,
			   pitchMod, lfo,
			   mProgram->feedback);
  }
}
