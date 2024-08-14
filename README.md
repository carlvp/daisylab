# daisylab
My experiments with the [Daisy Seed](https://github.com/electro-smith) audio dev board

* *test-tone* produces a 440 Hz test tone
* *midipod* a very simple synth for Daisy Pod. POT1=filter cutoff frequency, POT2=resonance, Mod Wheel=filter envelope modulation depth. Based on the Midi example program for Daisy Pod.
* *daisychain* experiments with I2S interface for digital audio. Apart from the I2S stuff, the demo program is just a simple tone generator and a delay line. See [demo on youtube](https://www.youtube.com/watch?v=YQDG7EdbL3k).
* *op6daisy* a six-operator FM synthesizer that can read/convert Yamaha DX7 bulk dumps. It won't emulate the real thing perfectly, but close enough for the patches to be quite useful anyway.
  [demo on youtube](https://www.youtube.com/watch?v=WLHoCTW1DcI).
* *op6-ui* user interface that manages *op6daisy patches*.
  * Facilitates loading bulk dumps (.syx)
  * Sends program change events to op6daisy.
  * Voice Editor, which allows the voice parameters to be modified.
  * See [demo on youtube](https://www.youtube.com/watch?v=ICCkKMRtbZw)
  Written in python, requires alsa_midi and tkinter. Runs well on Raspberry PI.
* *latebloomer* an attempt to emulate the classic LatelyBass program using a hacked version of op6daisy. Just 4 FM operators and the secret sauce: waveform #5 "half sine". Now with a resonant low-pass filter. See [demo on youtube](https://youtu.be/ICCkKMRtbZw?si=vZv6IJjyguxRiWcW&t=173) towards the end of the Voice Editor video.
