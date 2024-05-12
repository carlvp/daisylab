# daisylab
My experiments with the [Daisy Seed](https://github.com/electro-smith) audio dev board

* *test-tone* produces a 440 Hz test tone  
* *op6daisy* a six-operator FM synthesizer that can read/convert Yamaha DX7 bulk dumps. It won't be emulate the real thing, but patches are quite useful anyway.* *op6-ui* user interface that manages *op6daisy patches*. It facilitates loading bulk dumps (.syx) and sending program change events to op6daisy. Written in python, requires alsa_midi and tkinter. Runs well on Raspberry PI.
