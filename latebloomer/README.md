latebloomer: my stab at the classic Lately Bass of the Yamaha TX81z

* Starting out from the op6daisy code, reducing the number of operators to
  four per voice and adding waveform 5 ("half sine").
  
* Same layout on breadboard as op6daisy. Relevant pins on Daisy Seed:

  | Pin | Function |
  | --- | ---------------------------------------- |
  |  18 | Audio Out 1 |
  |  19 | Audio Out 2 |
  |  20 | Analog Ground |
  |  40 | Digital Ground (Analog and Digital Ground should be connected) |
  
* MIDI is passed over USB by default. USB over UART (MIDI port) is a
  configuration option (see configuration.h).
  
* Latebloomer listens to MIDI note-on/off, pitch-bend and controls 7
  (channel volume) and 10 (pan). It doesn't do program change. There is just
  a single program, Lately Bass. 

* Since it is based on op6daisy, which is a multi-timbral synth, it's possible
  to have several instances of the program on separate channels.

* Polyphony is 16 voices. It should be possible to crank that up quite a bit,
  given that each voice is just 4 operators (rather than 6). See NUM_VOICES
  in configuration.h. Keep your eyes on the LED, which indicates MIDI activity.
  It doubles as buffer underrun indicator. It's lit up permanently in the
  event of underrun. There will be underruns if the NUM_VOICES is increased
  beyond what the Daisy is able to deliver.
  
Hope you like it!
/Carl
