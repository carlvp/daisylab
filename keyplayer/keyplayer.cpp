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

KeyPlayer theKeyPlayer;

void KeyPlayer::Init() {
  // Basic initialization of Daisy hardware
  DaisySeedHw.Configure();
  DaisySeedHw.Init();

  // Tune the KeyPlayer:
  // Phase is represented as a 32-bit integer and 2PI corresponds to 2^32
  // The phase increment of a 1Hz signal is 2^32/sampleRate
  float sampleRate=DaisySeedHw.AudioSampleRate();
  mDeltaPhi1Hz=4294967296.0f/sampleRate;
  // In the same way for midi key A4 (440Hz)
  mDeltaPhiA4=440*mDeltaPhi1Hz;
}

int main(void)
{
  UsbMidiDispatcher midi;
  
  theKeyPlayer.Init();
  /* TODO: move this stuff into the Keyplayer singleton? */
  midi.Init();
  initVoices();
  
  startAudioPath();
  while(1) {
    processAudioPath();
    midi.Process();
  }
}
