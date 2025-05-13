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
        self.pgmBank=ProgramBank(NUM_PROGRAMS, OP6_BANKS_DIR)
        self.performanceController=PerformanceController()
        self.voiceSelectController=VoiceSelectController(self.pgmBank)
        self.voiceEditorController=VoiceEditorController(editBuffer,
                                                         self.pgmBank)
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
        # import program banks from persistent storage
        self.pgmBank.importPrograms()

        # MIDI connection
        midi=self.midiController.getMidiOut()
        self.performanceController.setMidiOut(midi)
        self.voiceSelectController.setMidiOut(midi)
        self.voiceEditorController.setMidiOut(midi)
        op6IsConnected=self.midiController.initModel()
        if op6IsConnected:
            self.initDevice_()

    def onConnectionChanged_(self, isConnected):
        '''called from the MidiController when op6 has been connected

           The call may be made from a thread, other than the main app
           thread (e.g. the MIDI-listener thread).'''
        callback=self.initDevice_ if isConnected else self.onDeviceDisconnect_
        self.view.postCallbackFromMain(callback)

    def initDevice_(self, arg=None):
        # send the current settings to the device
        self.midiController.getMidiOut().sendReset()
        self.voiceSelectController.syncProgramOnConnect()
        self.performanceController.syncPerformanceParametersOnConnect()
        # programs are transferred by means of the edit buffer
        oldOpMode=self.currOpMode
        self.setOpMode(EDIT_MODE, force_midi_tx=True)
        self.voiceEditorController.syncProgramsOnConnect(NUM_PROGRAMS)
        self.setOpMode(oldOpMode)
        self.performanceController.onConnectionChanged(True)

    def onDeviceDisconnect_(self, arg=None):
        self.performanceController.onConnectionChanged(False)

    def initUI(self):
        self.performanceController.initUI()
        self.voiceSelectController.initUI()

    def startUp(self):
        # start MIDI listener thread
        self.midiController.startUp(self.onConnectionChanged_)

    def shutDown(self):
        # stop the midi-listener thread, free MIDI resources
        self.midiController.shutDown()

    def setOpMode(self, mode, force_midi_tx=False):
        if self.currOpMode!=mode or force_midi_tx:
            self.midiController.setOperationalMode(mode)
            self.currOpMode=mode

    def setActiveScreen(self, screen):
        PERFORMANCE_SCREEN=0
        VOICE_SELECT_SCREEN=1
        VOICE_EDITOR_SCREEN=2

        mode=EDIT_MODE if screen==VOICE_EDITOR_SCREEN else PERFORMANCE_MODE
        if self.currOpMode!=mode:
            if mode==EDIT_MODE:
                self.voiceEditorController.prepareEditMode()
            self.setOpMode(mode)
        self.view.selectScreen(screen)

    def setClipboard(self, clipboard):
        '''set clipboard (or clear it using clipboard=None)'''
        if self.clipboard!=clipboard:
            self.clipboard=clipboard
            # TODO: notify all controllers
            # self.performanceController.clipboadChangedNotifier(clipboard)
