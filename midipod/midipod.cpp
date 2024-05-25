#include "daisy_pod.h"
#include "daisysp.h"
#include <stdio.h>
#include <string.h>

using namespace daisy;
using namespace daisysp;

DaisyPod   hw;
Oscillator osc;
AdEnv      env;
Svf        filt;
Parameter  knobCutoff, knobReso;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float sig, gain=0;
    for(size_t i = 0; i < size; i += 2)
    {
        sig = osc.Process();
        filt.Process(sig);
        gain=env.Process();
        out[i] = out[i + 1] = gain*filt.Low();
    }
}

static void noteOn(unsigned key, unsigned velocity) {
    osc.SetFreq(mtof(key));
    osc.SetAmp(velocity/127.0f);
    env.Trigger();
}

static void setCutoff(float freq) {
  filt.SetFreq(freq);
}

static void setReso(float reso) {
  filt.SetRes(reso);
}


// Typical Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            char        buff[512];
            sprintf(buff,
                    "Note Received:\t%d\t%d\t%d\r\n",
                    m.channel,
                    m.data[0],
                    m.data[1]);
            hw.seed.usb_handle.TransmitInternal((uint8_t *)buff, strlen(buff));
            // This is to avoid Max/MSP Note outs for now..
            if(m.data[1] != 0)
            {
                p = m.AsNoteOn();
                noteOn(p.note, p.velocity);
            }
        }
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
	  setCutoff(cutoff*16000.0f);
	  lastCutoff=cutoff;
	}
	float reso=knobReso.Process();
	if (fabs(reso-lastReso) > 0.0001) {
	  setReso(reso);
	  lastReso=reso;
	}
    }
}
