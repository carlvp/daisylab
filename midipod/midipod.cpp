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
        case ControlChange:
        {
            ControlChangeEvent p = m.AsControlChange();
            switch(p.control_number)
            {
                case 1:
                    // CC 1 for cutoff.
                    filt.SetFreq(mtof((float)p.value));
                    break;
                case 2:
                    // CC 2 for res.
                    filt.SetRes(((float)p.value / 127.0f));
                    break;
                default: break;
            }
            break;
        }
        default: break;
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
    for(;;)
    {
        hw.midi.Listen();
        // Handle MIDI Events
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
    }
}
