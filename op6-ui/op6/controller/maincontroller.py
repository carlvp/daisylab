from .performance import PerformanceController
from .voice import VoiceEditorController
from .midi import MidiController, EDIT_MODE, PERFORMANCE_MODE
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
        self.midiController=MidiController()
        self.clipboard=None
        self.currOpMode=PERFORMANCE_MODE

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

    def startUp(self):
        midi=self.midiController.getMidiOut()
        self.performanceController.setMidiOut(midi)
        self.voiceEditorController.setMidiOut(midi)
        self.midiController.startUp()
        
    def shutDown(self):
        # stop the midi-listener thread, free MIDI resources
        self.midiController.shutDown()
    
    def initUI(self):
        self.performanceController.initUI()

    def setActiveScreen(self, index):
        mode=index # mode 0, 1 same as screen index
        if self.currOpMode!=mode:
            if mode==EDIT_MODE:
                self.voiceEditorController.prepareEditMode()
            self.midiController.setOperationalMode(mode)
            self.currOpMode=mode
        self.view.selectScreen(index)

    def setClipboard(self, clipboard):
        '''set clipboard (or clear it using clipboard=None)'''
        if self.clipboard!=clipboard:
            self.clipboard=clipboard
            # TODO: notify all controllers
            # self.performanceController.clipboadChangedNotifier(clipboard)
