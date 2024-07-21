PARAMETERS_PER_OPERATOR=128

# interface (MainController):
# registerControllerObjects()
# setView()
# updateUI()
#
# interface (View):
# updateVoiceParameter()
#
# interface (ProgramChangeListener):
# notifyProgramChange()

class VoiceEditorController:
    '''
    The VoiceController manages the UI of the VoiceEditorScreen
    and mediates user interaction and operations in the voice editor
    '''

    def __init__(self, editBuffer):
        self.editBuffer=editBuffer
        self.disableParameterUpdates=False
        self.voiceEditorScreen=None
        self.screenIsUpToDate=False
        self.currSyx=None
        self.currProgram=0
        self.editBufferLoadedFrom=None
        self.midiOut=None
        self.baseChannel=0

    def registerControllerObjects(self, controllers):
        '''adds controller objects to the dictionary, controllers.'''
        controllers['VoiceEditorController']=self

    def setViews(self, views):
        '''connects to relevant views in the dictionary, views'''
        self.voiceEditorScreen=views['VoiceEditorScreen']

    def notifyBankChange(self, syx):
        '''Called when a new bank has been loaded'''
        self.screenIsUpToDate=False
        self.currSyx=syx

    def notifyProgramChange(self, program):
        '''Called at program change (invalidates edit buffer)'''
        self.screenIsUpToDate=False
        self.currProgram=program

    def prepareEditMode(self):
        '''Set up the Voice Edit Buffer on the MIDI device'''
        if (self.editBufferLoadedFrom is None
            or self.editBufferLoadedFrom!=self.currProgram):
            LOAD_BUFFER=7*128 + 2
            programAs14bit=self.currProgram*128;
            self.midiOut.sendParameter(self.baseChannel, LOAD_BUFFER, programAs14bit)
            self.editBufferLoadedFrom=self.currProgram

    def updateUI(self):
        '''Updates the UI before switching to the VoiceEditScreen'''
        # FIXME: switching back and forth between screens doesn't work
        # edited values aren't retained when switching back
        # midi device isn't reloaded with original values either
        # Further when changing program, changing back and re-entering edit mode
        # the mididevice still keeps the edited values
        if not self.screenIsUpToDate:
            # Update editBuffer
            if self.currSyx is None:
                self.editBuffer.setInitialVoice()
            else:
                self.editBuffer.loadFromSyx(self.currSyx.getVoice(self.currProgram))
            # Update VoiceEditScreen from editBuffer
            self.disableParameterUpdates=True
            self.voiceEditorScreen.setVoiceParameter("Voice Number", str(self.currProgram+1))
            for (name, value) in self.editBuffer.getAllVoiceParameters():
                if _isBaseOne(name):
                    value=value+1
                self.voiceEditorScreen.setVoiceParameter(name, str(value))
            self.disableParameterUpdates=False

    def updateVoiceParameter(self, paramName, paramValue):
        if self.disableParameterUpdates:
            # updates are disabled when this controller sets parameters
            # this is not critical, but it avoids unnecessary processing
            return

        if _isBaseOne(paramName) and paramValue!="":
            paramValue=int(paramValue)-1

        if paramName=="Voice Number":
            # The voice number is not part of the edit buffer
            # TODO: we will need to keep track of the voice number for store
            # TODO: Voice number is "base 1": "1" represents voice #0
            pass
        else:
            changed=self.editBuffer.setVoiceParameter(paramName, paramValue)
            if changed:
                self.editBuffer.sendVoiceParameter(paramName,
                                                   self.midiOut,
                                                   self.baseChannel)

    def setMidiOut(self, midiOut):
        self.midiOut=midiOut


def _isBaseOne(paramName):
    # Some integer parameters 0..N-1 are represented as 1..N in the UI
    return (paramName=="Voice Number" or paramName=="Algorithm")
