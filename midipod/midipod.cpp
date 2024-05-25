#include "daisy_pod.h"
#include "daisysp.h"
#include <stdio.h>
#include <string.h>

using namespace daisy;
using namespace daisysp;

static DaisyPod   hw;
static Oscillator osc;
static AdEnv      env;
static Svf        filt;
static Parameter  knobCutoff, knobReso;
static float filterCutoff, filterEnvModPlusOne;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float sig;
    for(size_t i = 0; i < size; i += 2)
    {
        float e=env.Process();
        float freq=filterCutoff*filterEnvModPlusOne*e;
        sig = osc.Process();
        filt.SetFreq(freq);
        filt.Process(sig);
        out[i] = out[i + 1] = e*filt.Low();
    }
}

static void noteOn(unsigned key, unsigned velocity) {
    osc.SetFreq(mtof(key));
    osc.SetAmp(velocity/127.0f);
    env.Trigger();
}

static void setCutoff(float c) {
    filterCutoff=8000*c;
}

static void setReso(float reso) {
    filt.SetRes(reso);
}

static void setEnvMod(float m) {
    filterEnvModPlusOne=m+1;
}

static void HandleNoteOnEvent(NoteOnEvent p) {
    if(p.velocity != 0) {
        noteOn(p.note, p.velocity);
    }
}

static void HandleControlChange(ControlChangeEvent p) {
    if (p.control_number==1) {
        // Mod Wheel
      setEnvMod(p.value*(8/127.0f));
    }
}

// Typical Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
    case NoteOn:
        HandleNoteOnEvent(m.AsNoteOn());
        break;
    case ControlChange:
        HandleControlChange(m.AsControlChange());
        break;
    default:
        break;
    }
}

// Main -- Init, and Midi Handling
int main(void)
{
    // Init
    float samplerate;
    hw.Init();
    hw.SetAudioBlockSize(4);
    hw.seed.usb_handle.Init(UsbHandle::FS_INTERNAL);
    System::Delay(250);
    knobCutoff.Init(hw.knob1, 0.0001f, 1, Parameter::EXPONENTIAL);
    knobReso.Init(hw.knob2, 0.0f, 1.0f, Parameter::LINEAR);


    // Synthesis
    samplerate = hw.AudioSampleRate();
    osc.Init(samplerate);
    osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    filt.Init(samplerate);
    env.Init(samplerate);
    env.SetTime(ADENV_SEG_ATTACK, .01f);
    env.SetTime(ADENV_SEG_DECAY, .4f);

    // Start stuff.
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    hw.midi.StartReceive();

    float lastCutoff=knobCutoff.Process();
    setCutoff(lastCutoff);
    setEnvMod(0);
    float lastReso=knobReso.Process();
    setReso(lastReso);
    for(;;) {
        hw.midi.Listen();
        // Handle MIDI Events
        if (hw.midi.HasEvents()) {
            HandleMidiMessage(hw.midi.PopEvent());
	}
	// Handle knobs
	float cutoff=knobCutoff.Process();
	if (fabs(cutoff-lastCutoff) > 0.0001) {
	  setCutoff(lastCutoff);
	  lastCutoff=cutoff;
	}
	float reso=knobReso.Process();
	if (fabs(reso-lastReso) > 0.0001) {
	  setReso(reso);
	  lastReso=reso;
	}
    }
}
