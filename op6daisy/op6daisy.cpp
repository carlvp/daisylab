/** Simple example of using USB MIDI 
 * 
 *  When the project boots up, a 100Hz sine wave will emit from both outputs,
 *  and the Daisy should appear as an Audio/MIDI device on a connected host.
 * 
 *  To keep the example short, only note on messages are handled, and there
 *  is only a single oscillator voice that tracks the most recent note message.
 */

#include "hardware.h"
#include "op6daisy.h"
#include "AudioPath.h"
#include "Instrument.h"
#include "MidiDispatcher.h"
#include "Program.h"

daisy::DaisySeed DaisySeedHw;

Op6Daisy theOp6Daisy;

static Instrument theInstrument;
static UsbMidiDispatcher theMidiDispatcher;

Op6Daisy::Op6Daisy() {
  // Tune the Op6Daisy:
  // Phase is represented as a 32-bit integer and 2PI corresponds to 2^32
  // The phase increment of a 1Hz signal is 2^32/sampleRate
  mDeltaPhi1Hz=4294967296.0f/SAMPLE_RATE;
  // In the same way for midi key A4 (440Hz)
  mDeltaPhiA4=440*mDeltaPhi1Hz;
}


int main(void)
{
  // Init
  DaisySeedHw.Configure();
  DaisySeedHw.Init();
  theInstrument.Init();
  theMidiDispatcher.Init(&theInstrument);

  // Main loop
  startAudioPath();
  while(1) {
    processAudioPath(&theInstrument);
    theMidiDispatcher.Process();
  }
}
