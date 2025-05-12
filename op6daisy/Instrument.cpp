#include <cstring>

#include "AudioPath.h"
#include "hardware.h"
#include "Instrument.h"
#include "Program.h"
#include "SyxBulkFormat.h"

#define NUM_DELAY_BUFFER_BLOCKS 1905
static int16_t delayBuffer[NUM_DELAY_BUFFER_BLOCKS*BLOCK_SIZE];

// Tune the Instrument:
// Phase is represented as a 32-bit integer and 2PI corresponds to 2^32
// The phase increment of a 1Hz signal is 2^32/sampleRate
// In the same way for midi key A4 (440Hz)
Instrument::Instrument()
  : mBaseChannel{0},
    mOperationalMode{kPerformanceMode},
    mCurrTimestamp{0},
    mSysExPtr{0},
    mWaitClearUnderrun{0},
    mDeltaPhi1Hz{4294967296.0f/SAMPLE_RATE},
    mDeltaPhiA4{4294967296.0f*440/SAMPLE_RATE},
    mLastTempProgram{nullptr},
    mSavedProgram{nullptr},
    mDelayFx{delayBuffer, NUM_DELAY_BUFFER_BLOCKS}
{
}

void Instrument::Init() {
  reset();
  memset(mTempRefCount, 0, sizeof(mTempRefCount));
}

void Instrument::resetAllControllers(unsigned ch) {
  mChannel[ch].resetAllControllers();
  mDataEntryRouting[ch]=0;
  memset(&mHiresControls[ch], 0, sizeof(unsigned short)*HiresCC::NUM_HIRES_CC);
}

#define MIN_DELAY_MS 4
#define MIN_DAMPING_HZ (131*64)
void Instrument::reset() {
  const Program *program1 = &mProgram[0];

  for (unsigned ch=0; ch<NUM_CHANNELS; ++ch) {
    mChannel[ch].reset(program1);
  }
  memset(mDataEntryRouting, 0, sizeof(mDataEntryRouting));
  memset(mHiresControls, 0, sizeof(mHiresControls));

  // reset delay
  mDelayFx.clearBuffer();
  mDelayFx.setDelayMs(MIN_DELAY_MS);
  mDelayFx.setFeedback(0);
  mDelayFx.setDamping(MIN_DAMPING_HZ);

  // All sound off (mVoices)
  // no: mBaseChannel
  // mOperationalMode
  // mCurrTimeStamp -proabably not
  // mSysExPtr
  // mWaitClearUnderrun
  // mLastTempProgram
  // (N)RPN business and DataEntry: mDataEntryRouting and mHiResControls
  // mVoiceEditBuffer
  // no: mProgram[], mTempPrograms[]
  // mTempRefCount[]
  // (?): mLastTempProgram
  // (?): mSavedProgram
  // no (?): mEditBuffer
  // no: mSysExBuffer
  // no: mMixer
  // other state
  mSysExPtr=0;
  setUnderrunLED(false);
  mWaitClearUnderrun=0;
}

void Instrument::fillBuffer(float *stereoOutBuffer) {
  mMixer.clearBuffers();

  for (Channel &ch: mChannel)
    ch.mixVoices(stereoOutBuffer, mMixer);

  // zero fill if there were no active channels
  if (mMixer.mixIsZero) {
    memset(stereoOutBuffer, 0, sizeof(float)*2*BLOCK_SIZE);
  }

  // Process effects
  mDelayFx.processBlock(stereoOutBuffer, mMixer.outputMix[Mixer::kDelayFx]);

  // Buffer is bound to underrun after loading a program bank.
  // But that's OK. Reset the LED after a little while.
  // This also means that (after a true underrun), the LED
  // is cleared when loading a new program bank
  if (mWaitClearUnderrun) {
    if (--mWaitClearUnderrun==0)
      setUnderrunLED(false);
  }
}
  
void Instrument::noteOn(unsigned ch, unsigned key, unsigned velocity) {
  // When editing voices, just allow note-on events on the base channel
  if (mOperationalMode==kPerformanceMode || ch==mBaseChannel) {
    Channel *channel=&mChannel[ch];
    Voice *voice=allocateVoice(ch, key);
    // Manage temporary programs (in Voice Edit Mode)
    const Program *oldProgram=releaseTempProgram(voice->getProgram());
    if (mOperationalMode==kEditMode) {
      Program *program=getTempProgram(oldProgram);
      channel->setProgram(program);
    }
    channel->noteOn(voice, key, velocity, mCurrTimestamp++);
  }
}

void Instrument::noteOff(unsigned ch, unsigned key) {
  // Don't filter: always send note-off if there is an active voice
  mChannel[ch].noteOff(key, mCurrTimestamp++);
}

void Instrument::controlChange(unsigned ch, unsigned cc, unsigned value) {
  // In general don't filter: there is no harm sending CC to unused channels
  // and doing so facilitates control logic when switching modes 
  Channel &channel=mChannel[ch];
  switch (cc) {
  case 1:
    channel.setModulationWheel(value*128);
    break;
  case 5:
    channel.setPortamentoTime(value*128);
    break;
  case 7:
    channel.setChannelVolume(value*128);
    break;
  case 10:
    channel.setPan(value*128);
    break;
  case 65:
    channel.setPortamentoMode((value<64)? PortamentoMode::Off :
			      (value<96)? PortamentoMode::Legato :
			                  PortamentoMode::AlwaysOn);
    break;
  case 6:
    controlChangeCoarse(ch, HiresCC::DataEntry, value);
    break;
  case 38:
    controlChangeFine(ch, HiresCC::DataEntry, value);
    break;
  case 94:
    // 50% max send level. a factor of two results from mono-to-stereo mix.
    channel.setFxSendLevel(Mixer::kDelayFx, value/256.0f);
    break;
  case 98:
    controlChangeFine(ch, HiresCC::NRPN, value);
    break;
  case 99:
    controlChangeCoarse(ch, HiresCC::NRPN, value);
    break;
  case 100:
    controlChangeFine(ch, HiresCC::RPN, value);
    break;
  case 101:
    controlChangeCoarse(ch, HiresCC::RPN, value);
    break;
  case 121:
    resetAllControllers(ch);
    break;
  case 126: /* mono */
  case 127: /* poly */
    /* TODO: all notes should automatically be turned off */
    channel.setPoly(cc==127);
    break;
  }
}

// Parameters, RPN=registered parameter number, NRPN=non-registered ditto
#define RPN_PREFIX        0x4000
#define NRPN_PREFIX       0x8000
#define PARAM_PREFIX_MASK 0xC000

#define PARAM_PITCH_BEND_SENSITIVITY (RPN_PREFIX | 0x0000)
#define PARAM_CHANNEL_FINE_TUNING    (RPN_PREFIX | 0x0001)
#define PARAM_CHANNEL_COARSE_TUNING  (RPN_PREFIX | 0x0002)
#define PARAM_MODULATION_DEPTH_RANGE (RPN_PREFIX | 0x0005)
#define PARAM_RPN_NULL               (RPN_PREFIX | 0x3fff)

#define PARAM_LSB_MASK    0x007f
#define PARAM_GET_PAGE(X) (((X) >> 7) & 0x7f)

enum NrpnPage {
  // First seven NRPN pages deal with the Voice Edit Buffer  
  kOp6Page, kOp5Page, kOp4Page, kOp3Page, kOp2Page, kOp1Page,
  kCommonVoicePage,
  // Then there are (other) system parameters
  kSystemPage,
  // and parameters, which are specific to a channel
  kChannelPage
};

void Instrument::controlChangeHires(unsigned ch, HiresCC cc, unsigned value) {
  mHiresControls[ch][cc]=value;

  switch (cc) {
  case DataEntry:
    setParameter(ch, mDataEntryRouting[ch], value);
    break;
  case NRPN:
    mDataEntryRouting[ch]=(NRPN_PREFIX | (value & 0x3fff));
    break;
  case RPN:
    mDataEntryRouting[ch]=(RPN_PREFIX | (value & 0x3fff));
    break;
  case NUM_HIRES_CC:
    break;
  }
}

static unsigned paramToCents(unsigned midiValue) {
  unsigned semi=midiValue>>7;    // msb = coarse [semis]
  unsigned cent=midiValue & 127; // lsb = find [cents]
  return 100*semi + cent;
}

void Instrument::setParameter(unsigned ch, unsigned paramNumber, unsigned value) {
  if ((paramNumber & PARAM_PREFIX_MASK) == RPN_PREFIX) {
    // Handle registered parameters
    switch (paramNumber) {
    case PARAM_RPN_NULL:
      // Data Entry is ignored until set to new, valid value 
      mDataEntryRouting[ch]=0;
      break;
    case PARAM_PITCH_BEND_SENSITIVITY:
      mChannel[ch].setPitchBendRange(paramToCents(value));
      break;
    case PARAM_MODULATION_DEPTH_RANGE:
      mChannel[ch].setModulationRange(value);
      break;
    case PARAM_CHANNEL_FINE_TUNING:
    case PARAM_CHANNEL_COARSE_TUNING:
    default:
      /* no action */
      break;
    }
  }
  else if ((paramNumber & PARAM_PREFIX_MASK) == NRPN_PREFIX) {
    unsigned page=PARAM_GET_PAGE(paramNumber);
    paramNumber &= PARAM_LSB_MASK;

    if (page<=kCommonVoicePage && ch==mBaseChannel) {
      // Voice Edit buffer ignores parameter changes unless on the base channel
      mVoiceEditBuffer.setParameter(page, paramNumber, value);
      // Voice Edit Buffer changed: mustn't recycle Last Temp Program
      mLastTempProgram=nullptr;
    }
    else if (page==kSystemPage && ch==mBaseChannel) {
      setSystemParameter(paramNumber, value);
    }
    else if (page==kChannelPage) {
      setChannelParameter(ch, paramNumber, value);
    }
  }
}

enum SystemParameter {
  kSwitchMode,
  kInitBuffer,
  kLoadProgramToBuffer,
  kStoreProgramFromBuffer,
  kDelayFeedback,
  kDelayTime,
  kDelayDamp
};

void Instrument::setSystemParameter(unsigned paramNumber, unsigned value) {
  unsigned msb=value>>7;
  
  switch (paramNumber) {
  case kSwitchMode:
    if (msb<NUM_OPERATIONAL_MODES)
	setOperationalMode(msb);
    break;
  case kInitBuffer:
    mVoiceEditBuffer.loadInitialProgram();
    mLastTempProgram=nullptr; // VoiceEditBuffer changed, reload Temp Program
    break;
  case kLoadProgramToBuffer:
    if (msb<NUM_PROGRAMS) {
      mVoiceEditBuffer.loadProgram(mProgram[msb]);
      mLastTempProgram=nullptr; // VoiceEditBuffer changed, reload Temp Program
    }
    break;
  case kStoreProgramFromBuffer:
    if (msb<NUM_PROGRAMS) {
      mVoiceEditBuffer.storeProgram(mProgram[msb]);
    }
    break;
  case kDelayFeedback:
    // 0...127/128 feedback
    mDelayFx.setFeedback(msb/128.0f);
    break;
  case kDelayTime:
    // 4, 10..1270 ms
    mDelayFx.setDelayMs(msb? msb*10 : MIN_DELAY_MS);
    break;
  case kDelayDamp:
    // 0..127 -> 8384 Hz..256Hz cutoff frequency
    mDelayFx.setDamping((131-msb)*64);
    break;
  }
}

void Instrument::setOperationalMode(unsigned mode) {
  if (mode!=mOperationalMode) {
    // save/restore the program on the base channel
    // when switching to/from Edit Mode
    if (mode==kEditMode) {
      mSavedProgram=mChannel[mBaseChannel].getProgram();
    }
    else {
      mChannel[mBaseChannel].setProgram(mSavedProgram);
    }
    mOperationalMode=static_cast<OperationalMode>(mode);
  }
}

// Non-registered parameters in Channel Page (8)
#define CHANNEL_PRESSURE_RANGE 0x0b

// Channel parameters for modulation routing #16...#47 in Channel Page (8)

// Modulation destinations
#define MOD_ROUTING_FIRST   0x10
#define MOD_DEST_LFO_PM     0x10
#define MOD_DEST_PITCH      0x18
#define MOD_DEST_LFO_AM     0x20
#define MOD_DEST_AMP_BIAS   0x28
#define MOD_ROUTING_LAST    0x2f

// Modulation sources
#define MOD_SRC_WHEEL       0
#define MOD_SRC_FOOT        1
#define MOD_SRC_BREATH      2
#define MOD_SRC_ATOUCH      3
#define MOD_SRC_MASK      0x7

static inline bool isModulationRouting(unsigned paramNumber) {
  return (paramNumber>=MOD_ROUTING_FIRST && paramNumber<=MOD_ROUTING_LAST);
}

static inline ModulationDestination getModulationDestination(unsigned paramNr) {
  return static_cast<ModulationDestination>((paramNr-MOD_ROUTING_FIRST)>>3);
}

static inline ModulationSource getModulationSource(unsigned paramNr) {
  // ModulationSource enum tags are bit-masks: 1, 2, 4, 8,...
  unsigned bitNumber=paramNr & MOD_SRC_MASK;
  return static_cast<ModulationSource>(1<<bitNumber);
}

void Instrument::setChannelParameter(unsigned channel,
				     unsigned paramNumber,
				     unsigned value) {
  unsigned msb=value>>7;

  if (isModulationRouting(paramNumber)) {
    ModulationSource src=getModulationSource(paramNumber);
    ModulationDestination dst=getModulationDestination(paramNumber);
    bool routingEnabled=(msb >= 64);
    mChannel[channel].setModulationRouting(src, dst, routingEnabled);
  }
  else {
    switch (paramNumber) {
    case CHANNEL_PRESSURE_RANGE:
      mChannel[channel].setChannelPressureRange(msb);
      break;
    }
  }
}

static const Program initVoice;

void Instrument::programChange(unsigned ch, unsigned p) {
  // In general, don't filter: there is no harm sending PC to unused channels
  // However, don't change the program of the base channel when editing voices
  if (mOperationalMode==kPerformanceMode || ch!=mBaseChannel) {
    const Program *program=(p>=NUM_PROGRAMS)?
      &initVoice : &mProgram[p];
  
    mChannel[ch].setProgram(program);
  }
}

void Instrument::channelPressure(unsigned ch, unsigned value) {
  mChannel[ch].setChannelPressure(value);
}

void Instrument::pitchBend(unsigned ch, int value) {
  // Don't filter: there is no harm sending PB to unused channels
  mChannel[ch].setPitchBend(value);
}

void Instrument::sysEx(const unsigned char *inBuffer, unsigned size) {
  // SysEx transfers are chopped up in fragments of 128 bytes
  // due to a limitation of the current MIDI Handler so we need
  // to glue the buffers back together

  if (!mSysExPtr && size>5) {
    // Start of new transfer, check if the message is for us
    static constexpr unsigned char YAMAHA_MANUFACTURER_ID=0x43;
    static constexpr unsigned char Packed32Voice=0x09;

    unsigned id=inBuffer[0];

    if (id==YAMAHA_MANUFACTURER_ID) {
      unsigned subStat_devNo=inBuffer[1];
      unsigned formatNo=inBuffer[2];
      unsigned byteCount=inBuffer[3]*128 + inBuffer[4];

      if (subStat_devNo==0 && formatNo==Packed32Voice && byteCount==0x1000) {
	// Start transfer to buffer
	mSysExBuffer[0]=0xf0;
	mSysExPtr=1;
      }
    }
  }

  if (mSysExPtr) {
    if (mSysExPtr+size<sizeof(mSysExBuffer)) {
      // glue together in buffers 
      memcpy(mSysExBuffer+mSysExPtr, inBuffer, size);
      mSysExPtr+=size;

      if (mSysExPtr==sizeof(SyxBulkFormat)-1) {
	mSysExBuffer[mSysExPtr]=0xf7; // End of SysEx
	loadSyxBulkFormat(reinterpret_cast<const SyxBulkFormat*>(mSysExBuffer));
	mSysExPtr=0; // Reset SysEx pointer (start over again)
	mWaitClearUnderrun=10; // Clear underrun LED after a little while
      }
    }
    else {
      // Error: reset SysEx pointer (start over again)
      mSysExPtr=0;
    }
  }
}

Program *Instrument::getTempProgram(const Program *tryThisNext) {
  Program *program=nullptr;
  unsigned i=NUM_VOICES;

  if (mLastTempProgram) {
    // First, use the last Temp Program (provided Edit Buffer hasn't changed)
    program=mLastTempProgram;
    i=program - mTempPrograms;
  }
  else if (isTempProgram(tryThisNext)) {
    // Second, check if proposed program is free
    unsigned j=tryThisNext - mTempPrograms;

    if (mTempRefCount[j]==0) {
      i=j;
      program=mTempPrograms+i;
    }
  }

  if (!program) {
    // Otherwise search for a free Temp Program
    for (i=0; i<NUM_VOICES; ++i)
      if (mTempRefCount[i]==0) {
	program=mTempPrograms+i;
	break;
      }
  }

  // since there are as many temp programs as there are voices
  // there will always be a temp program to recycle.
  assert_with_led(program && i<NUM_VOICES);

  mTempRefCount[i]++;
  if (program != mLastTempProgram) {
    // Voice Edit Buffer has changed and needs to be stored to temp program
    mVoiceEditBuffer.storeProgram(*program);
    mLastTempProgram=program;
  }
  return program;
}

Program *Instrument::releaseTempProgram(const Program *pgm) {
  if (isTempProgram(pgm)) {
    unsigned i=pgm - mTempPrograms;
    if (--mTempRefCount[i]==0)
      return mTempPrograms+i;
  }
  return nullptr; 
}

Voice *Instrument::allocateVoice(unsigned ch, unsigned key) {
  // First check if we already have an active voice on the channel
  // TODO: mute groups -doesn't have to be the same key, same group is OK
  Voice *result=mChannel[ch].allocateVoice(key);
  
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

void Instrument::loadSyxBulkFormat(const SyxBulkFormat *syx) {
  for (unsigned i=0; i<32; ++i) {
    mVoiceEditBuffer.loadSyx(syx->voiceParam[i]);
    mVoiceEditBuffer.storeProgram(mProgram[i]);
  }
  mVoiceEditBuffer.loadInitialProgram(); // don't leave state behind
  mLastTempProgram=nullptr; // Last Temp Program and buffer differ
}
