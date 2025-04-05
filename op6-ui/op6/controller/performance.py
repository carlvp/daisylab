# interface (MainController):
# registerModules()
# resolveModules()
# setMidiOut()
# initUI()
#
# interface (View)
# updatePerformanceParameter()

_CC_VOLUME=7
_CC_PAN=10

def _midi_transmit_cc(midi, channel, cc, value):
    midi.sendControlChange(channel, cc, value)

_performanceParameters = {
    # paramName -> (index, midi-nr, initial, transmit())
    "Volume": (0, _CC_VOLUME, 90, _midi_transmit_cc),
    "Pan":    (1, _CC_PAN,    64, _midi_transmit_cc),
}

_NUM_PERFORMANCE_PARAMETERS=2

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
