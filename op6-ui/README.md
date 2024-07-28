op6-ui python-based user interface for op6daisy FM synth

* Loads SysEx (.syx) files onto the Daisy Seed via MIDI over USB.
* Chops-up the .syx files into 128-byte chunks that can be handled
  by the current MIDI implementation in libDaisy.
* Lists the voice names on 32 buttons, which send MIDI program change
  when pressed.
* Voice Editor allows the voice parameters to be modified.
* Parameter changes are sent to Daisy as non-registered paramters (NRPN).

Usage: start app when Daisy Seed (op6daisy) is up and running (or it
won't connect to the Daisy). Start like so: python -m op6
(or similarly).

* Dependence tkinter and alsa_midi.
