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
  mOp.Init(sampleRate);
}

void Voice::noteOn(const Program *p, unsigned key, unsigned velocity) {
  mProgram=p;
  mKey=key;
  mNoteOn=true;
  mTimestamp=getAudioPathTimestamp();

  float freq=daisysp::mtof(key);
  mOp.noteOn(&p->op, freq, velocity);
}

void Voice::noteOff() {
  mNoteOn=false;
  mTimestamp=getAudioPathTimestamp();
  mOp.noteOff();
}

void Voice::addToBuffer(float *buffer) {
  mOp.fillBuffer(buffer, buffer, /* TBD (fm) */ nullptr);
}
