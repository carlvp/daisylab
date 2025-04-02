# interface (MainController):
# registerModules()
# resolveModules()
# setMidiOut()
#
# interface (VoiceEditorController)
# notifyBufferStored()
#
# interface (View)
# setVoice(voiceNumber)
# loadVoiceBank()

class PerformanceController:
    '''
    The PerformanceController manages the UI of the PerformanceScreen
    and mediates its user interaction and operations to the synthesizer
    over MIDI.
    '''

    def __init__(self):
        self.performanceScreen=None
        self.midiOut=None

    def registerModules(self, modules):
        '''adds this controller object to the module dictionary.'''
        modules['PerformanceController']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.performanceScreen=modules['PerformanceScreen']

    def setMidiOut(self, midiOut):
        self.midiOut=midiOut
