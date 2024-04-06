#include <cstring>

#include "AudioPath.h"
#include "Instrument.h"

void Instrument::fillBuffer(float *stereoOutBuffer) {
  const float *stereoMix=zeroBuffer;

  for (Channel ch: mChannel)
    stereoMix=ch.mixVoices(stereoOutBuffer, stereoMix);

  // zero fill if there were no active channels
  if (stereoMix==zeroBuffer)
    memset(stereoOutBuffer, 0, sizeof(float)*2*BLOCK_SIZE);
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
  if (cc==7)
    channel.setChannelVolume(value*128);
  else if (cc==10)
    channel.setPan(value*128);
}

void Instrument::programChange(unsigned ch, unsigned program) {
  mChannel[ch].setProgram(program);
}

void Instrument::pitchBend(unsigned ch, int value) {
  mChannel[ch].setPitchBend(value);
}

Voice *Instrument::allocateVoice(unsigned ch, unsigned key) {
  // First check if we already have am active voice on the channel
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
