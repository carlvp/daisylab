#include <cstring>

#include "AudioPath.h"
#include "hardware.h"
#include "Instrument.h"
#include "Program.h"
#include "SyxBulkFormat.h"

// Tune the Instrument:
// Phase is represented as a 32-bit integer and 2PI corresponds to 2^32
// The phase increment of a 1Hz signal is 2^32/sampleRate
// In the same way for midi key A4 (440Hz)
Instrument::Instrument()
  : mBaseChannel{0},
    mCurrTimestamp{0},
    mSysExPtr{0},
    mWaitClearUnderrun{0},
    mDeltaPhi1Hz{4294967296.0f/SAMPLE_RATE},
    mDeltaPhiA4{4294967296.0f*440/SAMPLE_RATE},
    mCurrCutoff{16383},
    mCurrReso{0}
{
}

void Instrument::Init() {
  const Program *program1 = &lateBloomer;

  for (unsigned ch=0; ch<NUM_CHANNELS; ++ch)
    mChannel[ch].reset(program1);
}

void Instrument::fillBuffer(float *stereoOutBuffer) {
  const float *stereoMix=zeroBuffer;

#ifdef WITH_FILTER_KNOBS
  unsigned short cutOff=getKnob(0);
  unsigned short reso=getKnob(1);
  
  if (cutOff!=mCurrCutoff) {
    mChannel[mBaseChannel].setFilterCutoff(cutOff);
    mCurrCutoff=cutOff;
  }
  if (reso!=mCurrReso) {
    mChannel[mBaseChannel].setFilterResonance(reso);
    mCurrReso=reso;
  }
#endif
  
  for (Channel &ch: mChannel)
    stereoMix=ch.mixVoices(stereoOutBuffer, stereoMix);

  // zero fill if there were no active channels
  if (stereoMix==zeroBuffer)
    memset(stereoOutBuffer, 0, sizeof(float)*2*BLOCK_SIZE);

  // Buffer is bound to underrun after a program bank.
  // But that's OK. Reset the LED after a little while.
  // This also means that (after a true underrun), the LED
  // is cleared after loading a new program bank
  if (mWaitClearUnderrun) {
    if (--mWaitClearUnderrun==0)
      setUnderrunLED(false);
  }
}

void Instrument::noteOn(unsigned ch, unsigned key, unsigned velocity) {
  Channel *channel=&mChannel[ch];
  Voice *voice=allocateVoice(ch, key);
  voice->noteOn(channel, key, velocity, mCurrTimestamp++);
}

void Instrument::noteOff(unsigned ch, unsigned key) {
  Voice *voice=mChannel[ch].findVoice(key);
  if (voice) {
    voice->noteOff(mCurrTimestamp++);
  }
}

void Instrument::controlChange(unsigned ch, unsigned cc, unsigned value) {
  Channel &channel=mChannel[ch];
  unsigned value14Bit=128*value;

  switch (cc) {
  case 7:
    channel.setChannelVolume(value14Bit);
    break;
  case 10:
    channel.setPan(value14Bit);
    break;
  case 71:
    channel.setFilterResonance(value14Bit);
    break;
  case 74:
    channel.setFilterCutoff(value14Bit);
    break;
  }
}

void Instrument::programChange(unsigned ch, unsigned p) {
  const Program *program=&lateBloomer;
  mChannel[ch].setProgram(program);
}

void Instrument::pitchBend(unsigned ch, int value) {
  mChannel[ch].setPitchBend(value);
}

void Instrument::sysEx(const unsigned char *inBuffer, unsigned size) {
  // sysEx support removed in Late Bloomer
}

Voice *Instrument::allocateVoice(unsigned ch, unsigned key) {
  // First check if we already have an active voice on the channel
  // TODO: mute groups -doesn't have to be the same key, same group is OK
  Voice *result=mChannel[ch].findVoice(key);
  
  if (!result) {
    // Voice stealing: look for the voice that has been released the longest
    result=&mVoice[0];

    unsigned maxAge=mCurrTimestamp-result->getTimestamp();
    bool gate=result->isNoteOn();
    
    for (unsigned i=1; i<NUM_VOICES; ++i) {
      Voice *v=&mVoice[i];
      unsigned age=mCurrTimestamp - v->getTimestamp();
      
      if (v->isNoteOn()) {
	if (gate && age>=maxAge) {
	  maxAge=age;
	  result=v;
	}
      }
      else if (gate || age>=maxAge) {
	gate=false;
	maxAge=age;
	result=v;
      }
    }
  }
  
  return result;
}

