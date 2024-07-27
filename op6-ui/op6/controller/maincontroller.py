from .performance import PerformanceController
from .voice import VoiceEditorController
from op6.model.editbuffer import EditBuffer

class MainController:
    '''
    The main controller manages user interaction and mediates operations 
    between view and model.
    '''
    def __init__(self):
        editBuffer=EditBuffer()
        self.voiceEditorController=VoiceEditorController(editBuffer)
        self.performanceController=PerformanceController()
        self.clipboard=None
        self.program=None
        self.midiOut=None
        self.baseChannel=0
        self.currOpMode=0

    def registerModules(self, modules):
        '''registers this module instance and those of possible submodules'''
        modules['MainController']=self
        self.performanceController.registerModules(modules)
        self.voiceEditorController.registerModules(modules)

    def resolveModules(self, modules):
        '''
        create connections to other modules (module is the dictionary
        that was created by registerModules).
        '''
        self.view=modules['MainView']
        self.performanceController.resolveModules(modules)
        self.voiceEditorController.resolveModules(modules)

    def initUI(self):
        self.performanceController.initUI()

    def setActiveScreen(self, index):
        self._setOperationalMode(index)
        self.view.selectScreen(index)

    def setClipboard(self, clipboard):
        '''set clipboard (or clear it using clipboard=None)'''
        if self.clipboard!=clipboard:
            self.clipboard=clipboard
            # TODO: notify all controllers
            # self.performanceController.clipboadChangedNotifier(clipboard)

    def setMidiOut(self, midiOut):
        self.midiOut=midiOut
        self.performanceController.setMidiOut(midiOut)
        self.voiceEditorController.setMidiOut(midiOut)

    def _setOperationalMode(self, mode):
        if self.currOpMode!=mode:
            SWITCH_MODE=7*128
            PERFORMANCE_MODE=0
            EDIT_MODE=1
            COMPARE_MODE=2
            if mode==EDIT_MODE:
                self.voiceEditorController.prepareEditMode()
            if self.midiOut is not None:
                self.midiOut.sendParameter(self.baseChannel,
                                           SWITCH_MODE,
                                           128*mode)
            self.currOpMode=mode
