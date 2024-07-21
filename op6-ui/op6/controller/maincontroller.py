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
        self.performanceController=PerformanceController(self.voiceEditorController)
        self.controllers={'MainController': self}
        self.performanceController.registerControllerObjects(self.controllers)
        self.voiceEditorController.registerControllerObjects(self.controllers)
        self.clipboard=None
        self.program=None
        self.midiOut=None
        self.baseChannel=0
        self.currOpMode=0

    def getControllers(self):
        '''returns a dictionary containing the controllers'''
        return self.controllers

    def setViews(self, views):
        '''
        sets the views of the various controllers
        views is a dictionary from controller name to instance
        '''
        self.view=views['MainView']
        self.performanceController.setViews(views)
        self.voiceEditorController.setViews(views)

    def initUI(self):
        self.performanceController.initUI()

    def setActiveScreen(self, index):
        if index==1:
            self.voiceEditorController.updateUI()
        self.view.selectScreen(index)
        self._setOperationalMode(index)

    def setClipboard(self, clipboard):
        '''
        set clipboard (or clear it using clipboard=None)
        '''
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
            self.midiOut.sendParameter(self.baseChannel, SWITCH_MODE, 128*mode)
            self.currOpMode=mode


