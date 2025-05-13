#include <cmath>
#include <cstring>
#include "AudioPath.h"
#include "Channel.h"
#include "Program.h"
#include "Voice.h"

static float monoMix[BLOCK_SIZE];

void Channel::resetAllControllers() {
  mExpression=16383/16384.0f; // (very close to) full volume
  setChannelVolume(90*128);   // about 50% linear gain (-6dB)
  setPan(8192);               // center
  mPitchWheel=0;              // no pitch bend
  setPitchBendRange(200);     // 200 cents
  mPoly=true;                 // Polyphonic operation
  mPortamentoMode=0;          // Portamento off
  setPortamentoTime(0);       // Instant pitch glide
  mModulationRange=16383;     // unity
  mModWheel=0;
  mChPressureRange=127;       // unity
  mChPressure=0;
  memset(mModRouting, 0, sizeof(mModRouting));
  mModRouting[ModulationDestination::LfoPmDepth]=ModulationSource::ModWheel;
  updateModWheel(false);      // don't propagate the update, already dealt with
  updateLfoPmDepth();
  updatePitchBend();
  updateLfoAmDepth();
  updateAmpBias();
  memset(mFxSendLevel, 0, sizeof(mFxSendLevel));
}

void Channel::reset(const Program *program) {
  mProgram=program;
  mNotesOn.clearAll();
  mLastKey=60;
  mLastKeyUp=true;
  resetAllControllers();
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
  else if (mProgram==voice->getProgram()) {
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
  mNotesOn.set(key);
  mLastKeyUp=(key>=mLastKey);
  mLastKey=key;
}

void Channel::noteOff(unsigned key, unsigned timestamp) {
  Voice *voice=findVoice(key);

  mNotesOn.reset(key);
  if (voice) {
    if (!mPoly && mNotesOn.any() && mProgram==voice->getProgram()) {
      // Monophonic mode: the voice is not released when another key is pressed
      // Keyboard priority (min/max) is determined by last interval (down/up).
      unsigned newKey=(mLastKeyUp)? mNotesOn.max() : mNotesOn.min();
      if (getPortamentoMode()!=PortamentoMode::Off)
	voice->setGlide(key-newKey);
      voice->changeKey(newKey);
    }
    else {
      voice->noteOff(timestamp);
    }
  }
}

// use zero buffer the first time, then outputMix
const float *Mixer::getInput(EffectBusName name) const {
  return mixIsZero? zeroBuffer : outputMix[name];
}

void Channel::mixVoicesPrivate(float *stereoMix, Mixer &mixer) {
  // Handle LFO
  float lfo=mLfo.sampleAndUpdate(mProgram->lfo);
  float pitchMod=exp2f(mLfoPmDepth*lfo + mPitchBendInOctaves);
  float ampMod=mLfoAmDepth*(1.0f-lfo) + mAmpBias;

  // Mix the channel's voices
  const float *monoIn=zeroBuffer;
  for (unsigned i=0; i<mNumVoices; ++i) {
    mVoice[i]->fillBuffer(monoMix, monoIn, pitchMod, ampMod);
    monoIn=monoMix;
  }

  // Pan and add to the stero mix and also effect bus
  const float *stereoIn=mixer.mixIsZero? zeroBuffer : stereoMix;
  float leftGain=mGain*mPanLeft;
  float rightGain=mGain*mPanRight;
  const float *delayIn=mixer.getInput(Mixer::kDelayFx);
  float *delayOut=mixer.outputMix[Mixer::kDelayFx];
  float delayLevel=mGain*mFxSendLevel[Mixer::kDelayFx];
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float x=monoMix[i];
    *stereoMix++ = *stereoIn++ + x*leftGain;
    *stereoMix++ = *stereoIn++ + x*rightGain;
    *delayOut++ = *delayIn++ + x*delayLevel;
  }
  // next voice mixes with the output of this one
  mixer.mixIsZero=false;
}

void Channel::setPan(unsigned p) {
  // -6dB equal power pan law: mLeftGain^2 + mRightGain^2 = 1
  float theta=(M_PI_2/16383)*p;
  mPanLeft=cosf(theta);
  mPanRight=sinf(theta);
  updateGain();
}

void Channel::setPitchBend(int b) {
  mPitchWheel=b;
  updatePitchBend();
}

void Channel::setPitchBendRange(unsigned cents) {
  // full pitch bend is +/-8192, a cent is 1/1200 octave
  // both scale factors are brought into mPitchBendRange
  mPitchBendRange=cents/(8192*1200.0f);
  if (mPitchWheel!=0) updatePitchBend();
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

void Channel::allSoundOff() {
  // TODO: this might be a bit crude. Turn off the voices less abruptly.
  for (unsigned i=0; i<mNumVoices; ++i)
    mVoice[i]->kill();
  mNumVoices=0;
  mNotesOn.clearAll();
}

void Channel::setProgram( const Program *program) {
  mProgram=program;
  // Update channel parameters, which depend on the program
  updateLfoPmDepth();
  updateLfoAmDepth();
  // No need to update PitchBend/AmpBias (no parameter in Program)
}

// Update modulation source Mod. Wheel when changed
void Channel::updateModWheel(bool propagate) {
  // Product value x range has 14+14=28 fractional bits
  mFromModWheel=ldexp(mModWheel*mModulationRange, -28);
  if (propagate) {
    // update modulation, which depends on modwheel
    updateDestinations(ModulationSource::ModWheel);
  }
}

void Channel::setChannelPressure(unsigned char value) {
  mChPressure=value;
  if (mChPressureRange!=0) updateDestinations(ModulationSource::ChannelPressure);
}

void Channel::setChannelPressureRange(unsigned char value) {
  mChPressureRange=value;
  if (mChPressure!=0) updateDestinations(ModulationSource::ChannelPressure);
}

// set modulation routing (on/off)
void Channel::setModulationRouting(ModulationSource src,
				   ModulationDestination dst,
				   bool routingEnabled) {
  unsigned char routing=mModRouting[dst];

  // set/clear routing-bit
  mModRouting[dst]=(routingEnabled)? (routing|src) : (routing & ~src);

  // update modulation depth
  switch (dst) {
  case LfoPmDepth: updateLfoPmDepth(); break;
  case LfoAmDepth: updateLfoAmDepth(); break;
  case AmpBias:    updateAmpBias(); break;
  case PitchBend:
  default:
    break;
  }
}

// Updates LFO pitch-modulation depth, when program, modulation settings
// and/or the modulator itself has changed
void Channel::updateLfoPmDepth() {
  if (mProgram!=nullptr) {
    float depth=mProgram->lfoPmInitDepth*(1.0f/99);
    unsigned char routing=mModRouting[ModulationDestination::LfoPmDepth];

    if (routing & ModulationSource::ModWheel) depth+=mFromModWheel;

    if (routing & ModulationSource::ChannelPressure) {
      depth+=mChPressure*mChPressureRange*(1.0f/127/127);
    }

    // Clamp depth to the interval [0, 1.0]
    if (depth>1.0f) depth=1.0f;
 
    mLfoPmDepth=depth*mProgram->lfoPmSensitivity;
  } else {
    mLfoPmDepth=0;
  }
}

// Updates pitch bend, s, when modulation settings and/or the modulator
// itself has changed.
void Channel::updatePitchBend() {
  float octaves=mPitchWheel*mPitchBendRange;
  unsigned char routing=mModRouting[ModulationDestination::PitchBend];

  if (routing & ModulationSource::ChannelPressure) {
    // full modulation range is 1270 cents, about 1 octave (127/120)
    octaves+=mChPressure*mChPressureRange*(1.0f/127/120);
  }

  mPitchBendInOctaves=octaves;
}

// Updates LFO amplitude-modulation depth, when program, modulation settings
// and/or the modulator itself has changed
void Channel::updateLfoAmDepth() {
  if (mProgram!=nullptr) {
    float depth=mProgram->lfoAmDepth;
    unsigned char routing=mModRouting[ModulationDestination::LfoAmDepth];

    // TODO: Just like AMD is superlinear in the range 89..99, we'd like
    // the am depth to increase more rapidly when mFromModWheel is close to 1.0
    if (routing & ModulationSource::ModWheel) depth+=mFromModWheel;

    if (routing & ModulationSource::ChannelPressure) {
      depth+=mChPressure*mChPressureRange*(1.0f/127/127);
    }

    mLfoAmDepth=depth;
  } else {
    mLfoAmDepth=0;
  }
}

// Updates amplitude bias, when modulation settings and/or the modulator
// itself has changed
void Channel::updateAmpBias() {
  float bias=0;
  unsigned char routing=mModRouting[ModulationDestination::AmpBias];

  // Bias is in negative logarithmic unit. [0,8] is a useful range.
  // 0 means 2^0 = 1.0 = 0 dB, higher numbers mean more attenuation.
  // 8.0 means almost silent, -48 dB
  if (routing & ModulationSource::ModWheel) {
    // Product has 14+14=28 bits, 3 integer bits, 25 fractional bits
    bias+=ldexpf((127*128-mModWheel)*mModulationRange, -25);
  }
  if (routing & ModulationSource::ChannelPressure) {
    // Product has 7+7=14 bits, interpreted as 3.11 for a range [0,8)
    bias+=(127-mChPressure)*mChPressureRange*(8.0f/127/128);
  }
  mAmpBias=bias;
}

void Channel::updateDestinations(ModulationSource sourceChanged) {
  if (mModRouting[ModulationDestination::LfoPmDepth] & sourceChanged)
    updateLfoPmDepth();
  if (mModRouting[ModulationDestination::PitchBend] & sourceChanged)
    updatePitchBend();
  if (mModRouting[ModulationDestination::LfoAmDepth] & sourceChanged)
    updateLfoAmDepth();
  if (mModRouting[ModulationDestination::AmpBias] & sourceChanged)
    updateAmpBias();
}
