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
_CC_DELAY_LEVEL=94
_CC_MONO=126
_CC_POLY=127

_RPN_PITCH_BEND_RANGE=0
_RPN_MODULATION_DEPTH_RANGE=5

_NRPN_SYSTEM_PAGE=7*128

_NRPN_DELAY_FEEDBACK=_NRPN_SYSTEM_PAGE+0x04
_NRPN_DELAY_TIME=_NRPN_SYSTEM_PAGE+0x05
_NRPN_DELAY_DAMP=_NRPN_SYSTEM_PAGE+0x06

_NRPN_CHANNEL_PAGE=8*128

_NRPN_MOD_RANGE=_NRPN_CHANNEL_PAGE+0x08
_NRPN_MOD_DEST_LFO_PM=_NRPN_CHANNEL_PAGE+0x10
_NRPN_MOD_DEST_PBEND=_NRPN_CHANNEL_PAGE+0x18
_NRPN_MOD_DEST_LFO_AM=_NRPN_CHANNEL_PAGE+0x20
_NRPN_MOD_DEST_AM_BIAS=_NRPN_CHANNEL_PAGE+0x28

_NRPN_CH_PRESS_RANGE=_NRPN_MOD_RANGE+3

_NRPN_MOD_WHEEL_LFO_PM=_NRPN_MOD_DEST_LFO_PM+0
_NRPN_CH_PRESS_LFO_PM=_NRPN_MOD_DEST_LFO_PM+3

_NRPN_CH_PRESS_PBEND=_NRPN_MOD_DEST_PBEND+3

_NRPN_MOD_WHEEL_LFO_AM=_NRPN_MOD_DEST_LFO_AM+0
_NRPN_CH_PRESS_LFO_AM=_NRPN_MOD_DEST_LFO_AM+3

_NRPN_MOD_WHEEL_AM_BIAS=_NRPN_MOD_DEST_AM_BIAS+0
_NRPN_CH_PRESS_AM_BIAS=_NRPN_MOD_DEST_AM_BIAS+3

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

def _midi_transmit_rpn_msb(midi, channel, rpn, value):
    midi.sendParameter(channel, rpn, 128*value, isRegistered=True)

def _midi_transmit_nrpn_on_off(midi, channel, nrpn, value):
    on_off=0 if value==0 else 127*128
    midi.sendParameter(channel, nrpn, on_off, isRegistered=False)

def _midi_transmit_nrpn_msb(midi, channel, rpn, value):
    midi.sendParameter(channel, rpn, 128*value, isRegistered=False)

_performanceParameters = {
    # paramName ->   (index, midi-nr, initial, transmit())
    "Volume":        (0, _CC_VOLUME,                   90, _midi_transmit_cc),
    "Pan":           (1, _CC_PAN,                      64, _midi_transmit_cc),
    "Poly":          (2, _CC_POLY,                      1, _midi_transmit_poly),
    "PortaTime":     (3, _CC_PORTA_TIME,                0, _midi_transmit_cc),
    "PortaMode":     (4, _CC_PORTAMENTO,                0, _midi_transmit_porta_mode),
    "PBendRange":    (5, _RPN_PITCH_BEND_RANGE,        20, _midi_transmit_pbendrange),
    "ModRange":      (6, _RPN_MODULATION_DEPTH_RANGE, 127, _midi_transmit_rpn_msb),
    "Mod2LfoPm":     (7, _NRPN_MOD_WHEEL_LFO_PM,        1, _midi_transmit_nrpn_on_off),
    "Mod2LfoAm":     (8, _NRPN_MOD_WHEEL_LFO_AM,        0, _midi_transmit_nrpn_on_off),
    "Mod2AmpBias":   (9, _NRPN_MOD_WHEEL_AM_BIAS,       0, _midi_transmit_nrpn_on_off),
    "PressRange":   (10, _NRPN_CH_PRESS_RANGE,        127, _midi_transmit_nrpn_msb),
    "Press2LfoPm":  (11, _NRPN_CH_PRESS_LFO_PM,         0, _midi_transmit_nrpn_on_off),
    "Press2PBend":  (12, _NRPN_CH_PRESS_PBEND,          0, _midi_transmit_nrpn_on_off),
    "Press2LfoAm":  (13, _NRPN_CH_PRESS_LFO_AM,         0, _midi_transmit_nrpn_on_off),
    "Press2AmpBias":(14, _NRPN_CH_PRESS_AM_BIAS,        0, _midi_transmit_nrpn_on_off),
    "DelayLevel":   (15, _CC_DELAY_LEVEL,               0, _midi_transmit_cc),
    "DelayFeedback":(16, _NRPN_DELAY_FEEDBACK,          0, _midi_transmit_nrpn_msb),
    "DelayTime":    (17, _NRPN_DELAY_TIME,              0, _midi_transmit_nrpn_msb),
    "DelayDamp":    (18, _NRPN_DELAY_DAMP,              0, _midi_transmit_nrpn_msb),
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
        self.resetPerformanceParameters()
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

    def resetPerformanceParameters(self, updateUI=False, sendMIDI=False):
        '''set performance parameters to initial default values'''
        for (name, (index, _, value, _)) in _performanceParameters.items():
            self.parameterValues[index]=value
        # TODO updateUI and sendMIDI, if True

    def initUI(self):
        '''reflect the performance parameters in the UI'''
        for (name, (index, _, value, _)) in _performanceParameters.items():
            self.performanceScreen.setPerformanceParameter(name, str(value))

    def syncPerformanceParametersOnConnect(self):
        '''sync a newly connected Op6 Daisy to current performance parameters

           the device is assumed to be in reset state (default values)'''
        for (name, (index, midiNr, initValue, midiTransmit)) in _performanceParameters.items():
            currValue=self.parameterValues[index]
            if currValue!=initValue:
                midiTransmit(self.midiOut, self.mBaseChannel, midiNr, currValue)

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

    def onConnectionChanged(self, isConnected):
        # indicate online status
        self.performanceScreen.setOnline(isConnected)
