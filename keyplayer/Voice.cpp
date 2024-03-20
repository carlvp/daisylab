#include "keyplayer.h"
#include "Voice.h"

Voice allVoices[NUM_VOICES];

void initVoices(float sampleRate) {
  for (Voice &v: allVoices)
    v.Init(sampleRate);
}

Voice *findVoice(unsigned channel, unsigned key) {
  for (Voice &v: allVoices)
    if (v.getKey()==key)
      return &v;

  return nullptr;
}

Voice *allocateVoice(unsigned channel, unsigned key) {
  Voice *v=findVoice(channel, key);
  unsigned timestamp=getAudioPathTimestamp();
  
  if (!v) {
    v=&allVoices[0];
    
    for (unsigned i=1; i<NUM_VOICES; ++i) {
      v = v->voiceStealing(&allVoices[i], timestamp);
    }
  }
  
  return v;
}

void Voice::Init(float sampleRate) {
  mOsc.Init(sampleRate);
  mEnv.Init(sampleRate);
}

void Voice::noteOn(unsigned key, unsigned velocity) {
  mKey=key;
  mNoteOn=true;
  mTimestamp=getAudioPathTimestamp();
  mOsc.SetFreq(daisysp::mtof(key));
  mEnv.SetTime(daisysp::ADENV_SEG_ATTACK, mProgram->attackTime);
  mEnv.SetTime(daisysp::ADENV_SEG_DECAY, mProgram->decayTime);
  mEnv.Trigger();
}

void Voice::noteOff() {
  mNoteOn=false;
  mTimestamp=getAudioPathTimestamp();
}

void Voice::addToBuffer(float *buffer) {
  for (unsigned i=0; i<BLOCK_SIZE; ++i) {
    float gain=mEnv.Process();
    
    mOsc.SetAmp(gain);
    buffer[i]+=mOsc.Process();
  }
}
