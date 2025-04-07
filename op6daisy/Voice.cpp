#include "Channel.h"
#include "FmAlgorithm.h"
#include "Instrument.h"
#include "Voice.h"
#include "Program.h"

void Voice::noteOn(Channel *ch, unsigned key, unsigned velocity, bool glide,
		   unsigned timestamp) {
  bool retrig=true;
  
  if (mChannel!=ch) {
    // Unregister with old channel
    if (mChannel)
      mChannel->removeVoice(this);
    
    // Register with new channel
    ch->addVoice(this);
    mChannel=ch;
  }
  else if (mKey==key) {
    // When the voice with the same channel and same key is recycled:
    // don't retrigger the envelopes.
    retrig=false;
  }
  else if (glide) {
    // Portamento: subtract (new key - old key)/12 from current CV
    // this makes the glide start from the current frequency
    mGlideCV -= (int) (key-mKey)*0.08333333f;
  }
  
  mProgram=ch->getProgram();
  mAlgorithm=FmAlgorithm::getAlgorithm(mProgram->algorithm);
  mKey=key;
  mGate=true;
  mTimestamp=timestamp;
  mEnvelope.noteOn(&mProgram->pitchEnvelope, 1.0f, 1.0f, retrig);
  
  std::int32_t deltaPhi=theOp6Daisy.midiToPhaseIncrement(key);
  float com=0.25f/mAlgorithm->getNumOutputs();
  for (unsigned i=0; i<NUM_OPERATORS; ++i) {
    float outputScaling=mAlgorithm->isOutput(i)? com : 1.0f;
    mOp[i].noteOn(&mProgram->op[i], key, velocity, deltaPhi, outputScaling, retrig);
  }
}

void Voice::noteOff(unsigned timestamp) {
  for (unsigned i=0; i<NUM_OPERATORS; ++i)
    mOp[i].noteOff(&mProgram->op[i]);
  mGate=false;
  mTimestamp=timestamp;
}

void Voice::fillBuffer(float *monoOut,
		       const float *monoIn,
		       float pitchMod,
		       float ampMod) {
  // Handle pitch envelope
  pitchMod*=mEnvelope.ProcessSample();
  mEnvelope.updateAfterBlock(&mProgram->pitchEnvelope);

  // Glide
  mGlideCV*=mChannel->getGlideDecayFactor();
  pitchMod*=exp2f(mGlideCV);
  
  if (mAlgorithm) {
    mAlgorithm->fillBuffer(monoOut, monoIn, mOp,
			   pitchMod, ampMod,
			   mProgram->feedback);
  }
}
