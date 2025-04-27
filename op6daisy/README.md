op6daisy is a six-operator FM synthesizer.

* It's straight-forward to set up on breadboard. Relevant pins on Daisy Seed:
  18 Audio Out 1
  19 Audio Out 2
  20 Analog Ground
  40 Digital Ground (Analog and Digital Ground should be connected)
  
* MIDI is passed over USB and Daisy can be powered over USB as well.
  In this set-up Daisy acts as a USB HID device.
  
* It is *not* a DX7 emulator, but sufficiently similar in voice
  architecture for it to be useful load DX7 SysEx (.syx) files
  in the format known as "32 voice bulk dump". Internally, the .syx
  is converted to op6daisys native format.
  
* Due to a current limitation of the implementation in libDaisy,
  the .syx files needs to be chopped up in pieces of at most 128 bytes
  of payload. There is a utility in this repository (op6-ui) that
  facilitates .syx import. Also part of op6-ui is a Voice Editor, by which
  voice parameters can be modified and new voices can be created from
  scratch.
  
* At power-up the INIT_VOICE is loaded. It's a rather dull sounding,
  single operator sine wave. With the Voice Editor (see op6-ui in this
  repository), new voices can be created. Another option is to import
  DX7 SysEx. Two good resources for .syx files:
  ** [http://bobbyblues.recup.ch/yamaha_dx7/dx7_patches.html]
  ** [https://yamahablackboxes.com/collection/yamaha-dx7-synthesizer/patches]
  
* In a addition to SysEx, the op6daisy listens to MIDI note-on/off,
  program change, pitch-bend, channel pressure and controls 1 (modulation
  wheel), 5 (portamento time), 7 (channel volume) 10 (pan), 11 (expression),
  65 (portamento), 126 and 127 (mono and poly operation). Registered parameter
  0 (pitch bend range) and 5 (modulation range) are supported.

* There are non-registered parameters for control of modulation depth and
  modulation routing (refer to the source code: Instrument.cpp and
  Channel.cpp).

* Unlike DX7, this is a multi-timbral synth. Each of the 16 MIDI channels
  can have a distinct program/patch/voice.

* Polyphony is now 16 voices (up from 4). It still seems possible to streamline
  the code a bit more, so perhaps the polyphony can be improved further. With
  6 FM operators per voice that is 6 x 16 = 96 FM Operators in total.
  That's 4.6M samples/s at 48kHz sample rate.

* This is work in progress, I hope to add to this list soon. But I think
  -despite its limitations- it's actually possible to use it as-is.

Hope you like it!
/Carl
