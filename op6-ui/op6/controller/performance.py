# interface (MainController):
# registerModules()
# resolveModules()
# setMidiOut()
# initUI()
#
# interface (View)
# updatePerformanceParameter()

_CC_PORTA_TIME=5
_CC_VOLUME=7
_CC_PAN=10
_CC_PORTAMENTO=65
_CC_MONO=126
_CC_POLY=127

_RPN_PITCH_BEND_RANGE=0

def _midi_transmit_cc(midi, channel, cc, value):
    midi.sendControlChange(channel, cc, value)

def _midi_transmit_poly(midi, channel, _, value):
    # mono (0) and poly (1) are represented by different MIDI CCs
    cc=_CC_MONO if value==0 else _CC_POLY
    midi.sendControlChange(channel, cc, 0)

def _midi_transmit_on_off(midi, channel, cc, value):
    on_off=0 if value==0 else 127
    midi.sendControlChange(channel, cc, on_off)

def _midi_transmit_porta_mode(midi, channel, cc, value):
    mode=(0 if value==0 else
          64 if value==1 else
          127)
    midi.sendControlChange(channel, cc, mode)

def _midi_transmit_pbendrange(midi, channel, _, value):
    # value is in units of 10 cents
    semi=value//10
    cent=10*value-100*semi
    midi.sendParameter(channel, _RPN_PITCH_BEND_RANGE, 128*semi+cent, isRegistered=True)
    
_performanceParameters = {
    # paramName -> (index, midi-nr, initial, transmit())
    "Volume":     (0, _CC_VOLUME,            90, _midi_transmit_cc),
    "Pan":        (1, _CC_PAN,               64, _midi_transmit_cc),
    "Poly":       (2, _CC_POLY,               1, _midi_transmit_poly),
    "PortaTime":  (3, _CC_PORTA_TIME,         0, _midi_transmit_cc),
    "PortaMode":  (4, _CC_PORTAMENTO,         0, _midi_transmit_porta_mode),
    "PBendRange": (5, _RPN_PITCH_BEND_RANGE, 20, _midi_transmit_pbendrange),
}

_NUM_PERFORMANCE_PARAMETERS=len(_performanceParameters)

class PerformanceController:
    '''
    The PerformanceController manages the UI of the PerformanceScreen
    and mediates its user interaction and operations to the synthesizer
    over MIDI.
    '''

    def __init__(self):
        self.performanceScreen=None
        self.parameterValues=[0 for index in range(_NUM_PERFORMANCE_PARAMETERS)]
        self.midiOut=None
        self.mBaseChannel=0

    def registerModules(self, modules):
        '''adds this controller object to the module dictionary.'''
        modules['PerformanceController']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.performanceScreen=modules['PerformanceScreen']

    def setMidiOut(self, midiOut):
        self.midiOut=midiOut

    def initUI(self):
        '''initializes the performance parameters and the View'''
        for (name, (index, _, value, _)) in _performanceParameters.items():
            self.parameterValues[index]=value
            self.performanceScreen.setPerformanceParameter(name, str(value))

    def updatePerformanceParameter(self, paramName, paramValue):
        '''called from view object when parameter changed'''
        try:
            value=int(paramValue)
        except ValueError:
            return False
        (index, midiNr, _, midiTransmit)=_performanceParameters[paramName]
        if self.parameterValues[index]!=value:
            self.parameterValues[index]=value
            midiTransmit(self.midiOut, self.mBaseChannel, midiNr, value)
            return True
        else:
            return False
