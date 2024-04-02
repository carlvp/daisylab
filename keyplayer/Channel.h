#pragma once
#ifndef Channel_H
#define Channel_H

class Program;

class Channel {
 public:
  Channel()
    : mMasterVolume(1.0f)
  {
    reset();
  }
  
  void reset();

  const Program *getProgram() const { return mProgram; }
  void setProgram(unsigned pgm);

  // Master Volume x Channel Volume x Expression [0,1.0]
  float getVolume() const { return mVolume; }

  void setMasterVolume(float v) {
    mMasterVolume=v;
    updateVolume();
  }
  
  // Channel Volume [0,16383]
  void setChannelVolume(unsigned v) {
    float r=v/16384.0f;
    mChannelVolume=r*r;
    updateVolume();
  }

  // Expression [0,16383]
  void setExpression(unsigned x) {
    mExpression=x/16384.0f;
    updateVolume();
  }

 private:
  const Program *mProgram;
  float mVolume, mMasterVolume, mChannelVolume, mExpression;

  void updateVolume() {
    mVolume=mMasterVolume*mChannelVolume*mExpression;
  }
};

#endif
