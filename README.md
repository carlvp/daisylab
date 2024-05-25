# daisylab
My experiments with the [Daisy Seed](https://github.com/electro-smith) audio dev board

* *test-tone* produces a 440 Hz test tone
* *midipod* a very simple synth for Daisy Pod. POT1=filter cutoff frequency, POT2=resonance, Mod Wheel=filter envelope modulation depth. Based on the Midi example program for Daisy Pod.
* *op6daisy* a six-operator FM synthesizer that can read/convert Yamaha DX7 bulk dumps. It won't emulate the real thing perfectly, but close enough for the patches to be quite useful anyway.
  [demo on youtube](https://www.youtube.com/watch?v=WLHoCTW1DcI).
* *op6-ui* user interface that manages *op6daisy patches*. It facilitates loading bulk dumps (.syx) and sending program change events to op6daisy. Written in python, requires alsa_midi and tkinter. Runs well on Raspberry PI.
