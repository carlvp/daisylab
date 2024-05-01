/** Simple example of using USB MIDI 
 * 
 *  When the project boots up, a 100Hz sine wave will emit from both outputs,
 *  and the Daisy should appear as an Audio/MIDI device on a connected host.
 * 
 *  To keep the example short, only note on messages are handled, and there
 *  is only a single oscillator voice that tracks the most recent note message.
 */

#include "hardware.h"
#include "AudioPath.h"
#include "Instrument.h"
#include "MidiDispatcher.h"
#include "Program.h"

daisy::DaisySeed DaisySeedHw;

Instrument theOp6Daisy;
static UsbMidiDispatcher theMidiDispatcher;

int main(void)
{
  // Init
  DaisySeedHw.Configure();
  DaisySeedHw.Init();
  theOp6Daisy.Init();
  theMidiDispatcher.Init(&theOp6Daisy);

  // Main loop
  startAudioPath();
  while(1) {
    processAudioPath(&theOp6Daisy);
    theMidiDispatcher.Process();
  }
}
