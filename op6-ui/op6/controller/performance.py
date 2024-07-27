import os
from op6.model.syx import SyxPacked32Voice

# interface (MainController):
# registerControllerObjects()
# setView()
# initUI()
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
    and mediates user interaction and operations in the performace mode
    '''

    def __init__(self):
        self.voiceEditor=None
        self.performanceScreen=None
        self.dialogManager=None
        self.currVoice=None
        self.baseChannel=0
        self.midiOut=None

    def registerModules(self, modules):
        '''adds this controller object to the module dictionary.'''
        modules['PerformanceController']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.voiceEditor=modules['VoiceEditorController']
        self.performanceScreen=modules['PerformanceScreen']
        self.dialogManager=modules['MainView']

    def initUI(self):
        self.currVoice=0
        self.performanceScreen.selectVoice(0)
        self.voiceEditor.notifyProgramChange(0)

    def setHasActiveScreen(self, hasActiveScreen):
        pass

    def setVoice(self, voiceNumber, notifyVoiceEditor=True):
        '''called from PerformanceScreen to set new voice'''
        self.performanceScreen.selectVoice(voiceNumber)
        self.currVoice=voiceNumber
        if self.midiOut:
            self.midiOut.sendProgramChange(self.baseChannel, voiceNumber)
        if notifyVoiceEditor:
            self.voiceEditor.notifyProgramChange(voiceNumber)

    def loadVoiceBank(self):
        '''called from the PerformanceScreen to load a voice bank'''
        filename=self.performanceScreen.askSyxFilename()
        if filename==() or filename=="":
            return # load cancelled
        
        syx=SyxPacked32Voice.load(filename, self.dialogManager)
        if syx is None:
            return # load failed (not a well-formed SyxPacked32Voice file)

        if self.midiOut:
            self.midiOut.chopUpSysEx(syx.getRawData(), 128)
        (bankName,_)=os.path.splitext(os.path.basename(filename))
        self.performanceScreen.setBankName(bankName[:24])
        for n in range(32):
            self.performanceScreen.setVoiceName(n, syx.getVoice(n).getName())
        self.voiceEditor.notifyBankChange(syx)
        self.voiceEditor.notifyProgramChange(self.currVoice)

    def setMidiOut(self, midiOut):
        self.midiOut=midiOut

    def notifyBufferStored(self, voiceNumber, voiceName):
        '''
        update voice name and (possibly) switch voiceNumber
        in response of the voice editor updating the voice parameters
        (and possibly the number as well)
        '''
        self.performanceScreen.setVoiceName(voiceNumber, voiceName)
        if self.currVoice!=voiceNumber:
            self.setVoice(voiceNumber, notifyVoiceEditor=False)
