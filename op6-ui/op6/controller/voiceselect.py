import os
from op6.model.syx import SyxPacked32Voice

# interface (MainController):
# registerModules()
# resolveModules()
# setMidiOut()
# initUI()
#
# interface (VoiceEditorController)
# notifyBufferStored()
#
# interface (View)
# setVoice(voiceNumber)
# loadVoiceBank()

class VoiceSelectController:
    '''
    The VoiceSelectController manages the UI of the VoiceSelectScreen
    and mediates user interaction and operations in the performace mode
    '''

    def __init__(self, programBank):
        self.voiceEditor=None
        self.voiceSelectScreen=None
        self.dialogManager=None
        self.currVoice=None
        self.baseChannel=0
        self.midiOut=None
        self.programBank=programBank

    def registerModules(self, modules):
        '''adds this controller object to the module dictionary.'''
        modules['VoiceSelectController']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.voiceEditor=modules['VoiceEditorController']
        self.voiceSelectScreen=modules['VoiceSelectScreen']
        self.dialogManager=modules['MainView']

    def initUI(self):
        for p in range(32):
            programName=self.programBank.getProgramName(p)
            self.voiceSelectScreen.setVoiceName(p, programName)
        self.currVoice=0
        self.voiceSelectScreen.selectVoice(0)
        self.voiceEditor.notifyProgramChange(0)

    def setHasActiveScreen(self, hasActiveScreen):
        pass

    def setVoice(self, voiceNumber, notifyVoiceEditor=True):
        '''called from VoiceSelectScreen to set new voice'''
        self.voiceSelectScreen.selectVoice(voiceNumber)
        self.currVoice=voiceNumber
        if self.midiOut:
            self.midiOut.sendProgramChange(self.baseChannel, voiceNumber)
        if notifyVoiceEditor:
            self.voiceEditor.notifyProgramChange(voiceNumber)

    def loadVoiceBank(self):
        '''called from the VoiceSelectScreen to load a voice bank'''
        filename=self.voiceSelectScreen.askSyxFilename()
        if filename==() or filename=="":
            return # load cancelled
        
        syx=SyxPacked32Voice.load(filename, self.dialogManager)
        if syx is None:
            return # load failed (not a well-formed SyxPacked32Voice file)

        if self.midiOut:
            self.midiOut.chopUpSysEx(syx.getRawData(), 128)
        (bankName,_)=os.path.splitext(os.path.basename(filename))
        self.voiceSelectScreen.setBankName(bankName[:24])
        for n in range(32):
            self.voiceSelectScreen.setVoiceName(n, syx.getVoice(n).getName())
        self.voiceEditor.notifyBankChange(syx)
        self.voiceEditor.notifyProgramChange(self.currVoice)

    def setMidiOut(self, midiOut):
        self.midiOut=midiOut

    def notifyBufferStored(self, voiceNumber):
        '''
        update voice name and (possibly) switch voiceNumber
        in response of the voice editor updating the voice parameters
        (and possibly the number as well)
        '''
        voiceName=self.programBank.getProgramName(voiceNumber)
        self.voiceSelectScreen.setVoiceName(voiceNumber, voiceName)
        if self.currVoice!=voiceNumber:
            self.setVoice(voiceNumber, notifyVoiceEditor=False)
