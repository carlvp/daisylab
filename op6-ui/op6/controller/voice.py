# interface (MainController):
# registerControllerObjects()
# setView()
# updateUI()
#
# interface (View):
# updateVoiceParameter()
# initVoiceEditor()
#
# interface (ProgramChangeListener):
# notifyProgramChange()
# notifyBanckChange()

NUM_PROGRAMS=32

class VoiceEditorController:
    '''
    The VoiceController manages the UI of the VoiceEditorScreen
    and mediates user interaction and operations in the voice editor
    '''

    def __init__(self, editBuffer):
        self.editBuffer=editBuffer
        self.programBank=[None]*NUM_PROGRAMS
        self.disableParameterUpdates=False
        self.voiceEditorScreen=None
        self.voiceIsUpToDate=False
        self.currProgram=0
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
        self.voiceIsUpToDate=False
        for i in range(32):
            self.editBuffer.loadFromSyx(syx.getVoice(i))
            self.programBank[i]=self.editBuffer.getVoiceParameters()

    def notifyProgramChange(self, program):
        '''Called at program change (invalidates edit buffer)'''
        self.voiceIsUpToDate=False
        self.currProgram=program

    def prepareEditMode(self):
        '''
        Set up the UI and Voice Edit Buffer on the MIDI device for Edit Mode
        '''
        if not self.voiceIsUpToDate:
            if self.programBank[self.currProgram] is None:
                self.initVoiceEditor()
            else:
                self._loadCurrProgram()
            self.voiceIsUpToDate=True

    def initVoiceEditor(self):
        '''initialize UI and voice editor'''
        INIT_BUFFER=7*128 + 1

        self.editBuffer.setInitialVoice()
        if self.midiOut is not None:
            self.midiOut.sendParameter(self.baseChannel, INIT_BUFFER, 0)
        self._updateUI()

    def _loadCurrProgram(self):
        '''load a program into the edit buffer and update UI'''
        LOAD_BUFFER=7*128 + 2

        pgm=self.programBank[self.currProgram]
        self.editBuffer.setVoiceParameters(pgm)
        if self.midiOut is not None:
            program14bit=self.currProgram*128
            self.midiOut.sendParameter(self.baseChannel,
                                       LOAD_BUFFER,
                                       program14bit)
        self._updateUI()

    def _updateUIField(self, name, value):
        if _isBaseOne(name):
            value=value+1
        self.voiceEditorScreen.setVoiceParameter(name, str(value))

    def requestUIFieldUpdate(self, paramName):
        '''Updates a single field of the UI when it loses focus'''
        value=self.editBuffer.getVoiceParameter(paramName)
        self._updateUIField(paramName, value)

    def _updateUI(self):
        '''Updates the UI before switching to the VoiceEditScreen'''
        old=self.disableParameterUpdates
        self.disableParameterUpdates=True
        self.voiceEditorScreen.setVoiceParameter("Voice Number", str(self.currProgram+1))
        for (name, value) in self.editBuffer.getAllVoiceParameters():
            self._updateUIField(name, value)
        self.disableParameterUpdates=old
        
    def updateVoiceParameter(self, paramName, paramValue):
        '''called from view object when parameter changed'''
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
                # special tweak for frequency mode "fixed" -> "ratio",
                # which may render an out-of-range ratio
                if paramName[4:]=="Frequency Mode" and int(paramValue)==0:
                    # parmeterName[:13]="Opx Frequency"
                    self._checkFrequencyRatio(paramName[:13])

                if self.midiOut is not None and paramName!="Voice Name":
                    self.editBuffer.sendVoiceParameter(paramName,
                                                       self.midiOut,
                                                       self.baseChannel)

    def _checkFrequencyRatio(self, paramName):
        '''tweak to put frequency in range when shifting to ratio mode'''
        freq=self.editBuffer.getVoiceParameter(paramName[:13])
        if freq>99.999:
            self.editBuffer.setVoiceParameter(paramName[:13], "99.999")
            old=self.disableParameterUpdates
            self.disableParameterUpdates=True
            self._updateUIField(paramName, "99.999")
            self.disableParameterUpdates=old

    def setMidiOut(self, midiOut):
        self.midiOut=midiOut

def _isBaseOne(paramName):
    # Some integer parameters 0..N-1 are represented as 1..N in the UI
    return (paramName=="Voice Number" or paramName=="Algorithm")
