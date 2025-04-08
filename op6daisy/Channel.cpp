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
  mPoly=true;                 // Polyphonic operation
  setPortamentoTime(0);       // Instant pitch glide
}

void Channel::addVoice(Voice *v) {
  if (mNumVoices<NUM_VOICES) {
    mVoice[mNumVoices]=v;
    mNumVoices++;
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

void Channel::noteOn(Voice *voice,
		     unsigned key,
		     unsigned velocity,
		     unsigned timestamp) {
  Channel *oldChannel=voice->getChannel();
  bool retrig=true;

  if (oldChannel!=this) {
    // A voice, which was associate with another channel (or no channel)
    // is used. Move it to this channel.
    if (oldChannel)
      oldChannel->removeVoice(voice);
    addVoice(voice);
  }
  else {
    // Recycling of a voice, which is already associated with this channel.
    unsigned oldKey=voice->getKey();
    if (key==oldKey)
      retrig=false; // Even same key: don't retrigger the envelopes
    else {
      // In legato mode, just note played "legato" (overlapping) result in
      // portamento
      PortamentoMode mode=getPortamentoMode();
      if (mode==PortamentoMode::AlwaysOn ||
	  (mode==PortamentoMode::Legato && voice->isNoteOn())) {
	// Glide the difference between the old and the new key, which
	// cause the portamento to start from the current pitch
	voice->setGlide(oldKey-key);
      }
    }
  }
  voice->noteOn(this, key, velocity, retrig, timestamp);
}

void Channel::noteOff(unsigned key, unsigned timestamp) {
  Voice *voice=findVoice(key);
  // TODO: in monophonic mode, glide back to any key still pressed
  if (voice) {
    voice->noteOff(timestamp);
  }
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

  // Pan and add to the stero mix
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float x=monoMix[i];
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

void Channel::setPortamentoTime(unsigned t) {
  // Portamento Time [0,16383]
  //  0    7 blocks (5ms)  x=-1.000 coeff=0.500
  //  1    8                 -0.875       0.545
  //  ...
  //  54  61                 -0.115       0.923539
  //  55  62       (41ms)    -0.113       0.924725
  // -------------------------------------------
  //  56  64       (43ms)    -0.109       0.926989
  //  57  69.8     (47ms)    -0.100       0.932840
  //  ...
  // 126 27500    (18.4s)    -0.000254    0.999824
  // 127 30000    (20.0s)    -0.000233    0.999839
  unsigned msb=t>>7;
  float x;
  if (msb<56) {
    x = -7.0f/(msb+7);
  }
  else {
    // Constants timeK[0]*2^(-msb/8)
    static const float timeK[8]={
      -14.00000f, -12.8381f, -11.7725f, -10.79548f,
       -9.899494f, -9.07787f, -8.32444f, -7.633554f
    };
    unsigned index=msb & 7;
    int exp=msb>>3;
    x=ldexpf(timeK[index], -exp);
  }
  mGlideDecayFactor=exp2f(x);
}
