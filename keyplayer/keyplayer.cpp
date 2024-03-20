/** Simple example of using USB MIDI 
 * 
 *  When the project boots up, a 100Hz sine wave will emit from both outputs,
 *  and the Daisy should appear as an Audio/MIDI device on a connected host.
 * 
 *  To keep the example short, only note on messages are handled, and there
 *  is only a single oscillator voice that tracks the most recent note message.
 */
#include <daisy_seed.h>
#include <daisysp.h>

#include "keyplayer.h"
#include "MidiDispatcher.h"
#include "Voice.h"

using namespace daisy;
using namespace daisysp;

DaisySeed DaisySeedHw;

static UsbMidiDispatcher midi;

int main(void)
{
  /* Basic initialization of Daisy hardware */
  DaisySeedHw.Configure();
  DaisySeedHw.Init();
  /* Initialize USB Midi */ 
  midi.Init();
  /* Initialize the voices and start the audio callback */
  initVoices(DaisySeedHw.AudioSampleRate());
  startAudioPath();
  while(1) {
    processAudioPath();
    midi.Process();
  }
}
