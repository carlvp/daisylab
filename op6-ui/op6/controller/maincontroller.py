from .performance import PerformanceController
from .voiceselect import VoiceSelectController
from .voiceeditor import VoiceEditorController
from .midi import MidiController, EDIT_MODE, PERFORMANCE_MODE
from .display import DisplayController
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
        self.screenControllers=[self.performanceController,
                                self.voiceSelectController,
                                self.voiceEditorController]
        self.activeController=None
        self.midiController=MidiController()
        self.clipboard=None
        self.currOpMode=PERFORMANCE_MODE
        self.displayController=DisplayController()

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
            self.initDevice()

    def initDevice(self, arg=None):
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

    def onDeviceDisconnect(self, arg=None):
        self.performanceController.onConnectionChanged(False)

    def initUI(self):
        self.performanceController.initUI()
        self.voiceSelectController.initUI()

    def startUp(self):
        # start MIDI listener thread
        midiEventListener=MidiEventListener(self.view.postCallbackFromMain,
                                            self,
                                            self.performanceController,
                                            self.voiceSelectController)
        self.midiController.startUp(midiEventListener)

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
        if self.activeController is not None:
            self.activeController.setDisplay(None)
        self.activeController=self.screenControllers[screen]
        self.activeController.setDisplay(self.displayController)
        self.view.selectScreen(screen)

    def setClipboard(self, clipboard):
        '''set clipboard (or clear it using clipboard=None)'''
        if self.clipboard!=clipboard:
            self.clipboard=clipboard
            # TODO: notify all controllers
            # self.performanceController.clipboadChangedNotifier(clipboard)

class MidiEventListener:
    def __init__(self,
                 postCallbackFromMain,
                 mainController,
                 performanceController,
                 voiceSelectController):
        self.postCallbackFromMain=postCallbackFromMain
        self.mainController=mainController
        self.performanceController=performanceController
        self.voiceSelectController=voiceSelectController

    def onConnectionChanged(self, isConnected):
        '''called from the MidiController when op6 has been connected

           The call may be made from a thread, other than the main app
           thread (e.g. the MIDI-listener thread).'''
        callback=(self.mainController.initDevice if isConnected
                  else self.mainController.onDeviceDisconnect)
        self.postCallbackFromMain(callback)

    def onProgramChange(self, ch, pgm):
        callback=self.voiceSelectController.onMidiProgramChange
        self.postCallbackFromMain(callback, (ch, pgm))

    def onControllerChange(self, ch, cc, value):
        callback=self.performanceController.onMidiControllerChange
        self.postCallbackFromMain(callback, (ch, cc, value))
