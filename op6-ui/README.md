op6-ui python-based user interface for op6daisy FM synth

Demo on youtube: [https://www.youtube.com/watch?v=ICCkKMRtbZw]

Three screens:
* Performance Screen - real-time MIDI controls, modulation routing
  and other settings.
* Voice Selection - lists the voice names on 32 buttons. Sends MIDI
  program change events when a new voice (MIDI program) is selected.
* Voice Editor - allows the voice parameters to be modified: either
  from scratch or starting from an existing voice. The layout of
  this screen is inspired by the "DX7 Voice Data List", the sheets
  of paper on which the voice parameters were documented back in the
  day.

Features:
* Loads Yamaha DX7 SysEx (.syx) files onto the Daisy Seed.
* Parameter changes are sent to Daisy as MIDI Control messages,
  registered parameters (RPN) and non-registered ditto (NRPN).
* MIDI hardware devices are automatically connected via the system
  MIDI-through. The Daisy itself is hotplugged when (re-)connected to
  the UI host.
* Channel volume (CC#7), pan (CC#10), portamento (CC#65), portamento
  time (CC#5), mono and poly operations (CCs #126 and #127) are
  reflected in the user interface.
* It is possible to set the depth of pitch bend, the modulation wheel
  and channel aftertouch.
* Modulation sources can be routed to pitch- and amplitude modulation
  depth (LFO modulation depth), pitch bend and amplitude bias.
* Persistent storage of voice parameters. They are kept in json files
  (see op6-banks subdirectory) and re-loads at next start-up.

Start like so: python -m op6 (or similarly).

* Dependence: tkinter and alsa_midi. Runs well on Raspberry PI.
