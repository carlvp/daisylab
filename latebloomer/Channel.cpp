#include <cmath>
#include "AudioPath.h"
#include "Channel.h"
#include "Program.h"
#include "Voice.h"

static float monoMix[BLOCK_SIZE];

void Channel::reset(const Program *program) {
  mProgram=program;
  mExpression=16383/16384.0f; // (very close to) full volume
  setChannelVolume(90*128);   // about 50% linear gain (-6dB)
  setPan(8192);               // center
  mPitchBendFactor=1.0f;      // no pitch bend
  setPitchBendRange(200);     // 200 cents ~ +/-2 semitones
  setFilterCutoff(16383);     // Filter is fully open
  setFilterResonance(0);      // No resonance
}

void Channel::addVoice(Voice *v) {
  if (mNumVoices<NUM_VOICES) {
    mVoice[mNumVoices]=v;
    mNumVoices++;
  }
  
  if (mNumVoices==1) {
    // Sync lfo at first note-on
    // FIXME: condition for re-trig is overly simple, what if other voice
    // has decayed to nearly zero..?
    // TODO: LfoParam should be part of Program
    mLfo.sync(mProgram->lfo);
  }
}

void Channel::removeVoice(Voice *v) {
  unsigned i;

  for (i=0; i<mNumVoices; ++i)
    if (mVoice[i]==v) {
      mNumVoices--;
      mVoice[i]=mVoice[mNumVoices];
      break;
    }
}

Voice *Channel::findVoice(unsigned key) const {
  for (unsigned i=0; i<mNumVoices; ++i)
    if (mVoice[i]->getKey()==key)
      return mVoice[i];
  
  return nullptr;
}

void Channel::mixVoicesPrivate(float *stereoMix, const float *stereoIn) {
  // Handle LFO
  float lfo=mLfo.sampleAndUpdate(mProgram->lfo);
  float lfoPmDepth=mProgram->lfoPmDepth;
  float pitchMod=exp2f(lfoPmDepth*lfo)*mPitchBendFactor;
  float lfoAmDepth=mProgram->lfoAmDepth;
  float ampMod=lfoAmDepth*(1.0f-lfo);

  
  // Mix the channel's voices
  const float *monoIn=zeroBuffer;
  for (unsigned i=0; i<mNumVoices; ++i) {
    mVoice[i]->fillBuffer(monoMix, monoIn, pitchMod, ampMod);
    monoIn=monoMix;
  }

  // Run through filter, pan and add to the stero mix
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float x=monoMix[i];
    mFilter.Process(x);
    x=mFilter.Low();
    stereoMix[2*i] = stereoIn[2*i] + x*mLeftGain;
    stereoMix[2*i+1] = stereoIn[2*i+1] + x*mRightGain;
  }
}

void Channel::setPan(unsigned p) {
  // -6dB equal power pan law: mLeftGain^2 + mRightGain^2 = 1
  float theta=(M_PI_2/16383)*p;
  mPanLeft=cosf(theta);
  mPanRight=sinf(theta);
  updateGain();
}

void Channel::setPitchBend(int b) {
  mPitchBendFactor=exp2(b*mPitchBendRange);
}

void Channel::setPitchBendRange(unsigned cents) {
  // full pitch bend is +/-8192, a cent is 1/1200 octave
  // both scale factors are brought into mPitchBendRange
  mPitchBendRange=cents/(8192*1200.0f);
}

void Channel::setFilterCutoff(unsigned v) {
  // msb (bits 7-13): is in semitones, counted from C0
  // lsb (bist 0-6): is in the unit of 1/128 semi tone (about one cent)
  constexpr float unitsPerOctave=12*128;
  constexpr float C0=16.3516f;
  float freq=C0*exp2f(v/unitsPerOctave);
  mFilter.SetFreq(freq);
}
