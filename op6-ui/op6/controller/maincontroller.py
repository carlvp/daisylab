from .performance import PerformanceController
from .voiceselect import VoiceSelectController
from .voiceeditor import VoiceEditorController
from .midi import MidiController, EDIT_MODE, PERFORMANCE_MODE
from op6.model.editbuffer import EditBuffer
from op6.model.programbank import ProgramBank

NUM_PROGRAMS=32
OP6_BANKS_DIR='op6-banks'

class MainController:
    '''
    The main controller manages user interaction and mediates operations 
    between view and model.
    '''
    def __init__(self):
        editBuffer=EditBuffer()
        pgmBank=ProgramBank(NUM_PROGRAMS, OP6_BANKS_DIR) 

        self.performanceController=PerformanceController()
        self.voiceSelectController=VoiceSelectController(pgmBank)
        self.voiceEditorController=VoiceEditorController(editBuffer, pgmBank)
        self.midiController=MidiController()
        self.clipboard=None
        self.currOpMode=PERFORMANCE_MODE

    def registerModules(self, modules):
        '''registers this module instance and those of possible submodules'''
        modules['MainController']=self
        self.performanceController.registerModules(modules)
        self.voiceSelectController.registerModules(modules)
        self.voiceEditorController.registerModules(modules)

    def resolveModules(self, modules):
        '''
        create connections to other modules (module is the dictionary
        that was created by registerModules).
        '''
        self.view=modules['MainView']
        self.performanceController.resolveModules(modules)
        self.voiceSelectController.resolveModules(modules)
        self.voiceEditorController.resolveModules(modules)

    def initModel(self):
        # MIDI connection
        midi=self.midiController.getMidiOut()
        self.performanceController.setMidiOut(midi)
        self.voiceSelectController.setMidiOut(midi)
        self.voiceEditorController.setMidiOut(midi)
        self.midiController.initModel()

        # import program banks from persistent storage, send to daisy
        self.voiceEditorController.importPrograms()

    def initUI(self):
        self.performanceController.initUI()
        self.voiceSelectController.initUI()

    def startUp(self):
        # start MIDI listener thread
        self.midiController.startUp()

    def shutDown(self):
        # stop the midi-listener thread, free MIDI resources
        self.midiController.shutDown()
    
    def setActiveScreen(self, screen):
        PERFORMANCE_SCREEN=0
        VOICE_SELECT_SCREEN=1
        VOICE_EDITOR_SCREEN=2

        mode=EDIT_MODE if screen==VOICE_EDITOR_SCREEN else PERFORMANCE_MODE
        if self.currOpMode!=mode:
            if mode==EDIT_MODE:
                self.voiceEditorController.prepareEditMode()
            self.midiController.setOperationalMode(mode)
            self.currOpMode=mode
        self.view.selectScreen(screen)

    def setClipboard(self, clipboard):
        '''set clipboard (or clear it using clipboard=None)'''
        if self.clipboard!=clipboard:
            self.clipboard=clipboard
            # TODO: notify all controllers
            # self.performanceController.clipboadChangedNotifier(clipboard)
