# interface (MainController):
# registerModules()
# resolveModules()
# setMidiOut()
#
# interface (View):
# updateVoiceParameter()
# initVoiceEditor()
# saveVoice()
#
# interface (ProgramChangeListener):
# notifyProgramChange()
# notifyBankChange()

class VoiceEditorController:
    '''
    The VoiceController manages the UI of the VoiceEditorScreen
    and mediates user interaction and operations in the voice editor
    '''

    def __init__(self, editBuffer, programBank):
        self.editBuffer=editBuffer
        self.programBank=programBank
        self.disableParameterUpdates=False
        self.voiceSelectController=None
        self.voiceEditorScreen=None
        self.voiceIsUpToDate=False
        self.currProgram=0
        self.midiOut=None
        self.baseChannel=0

    def registerModules(self, modules):
        '''adds this controller object to the module dictionary.'''
        modules['VoiceEditorController']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.voiceSelectController=modules['VoiceSelectController']
        self.voiceEditorScreen=modules['VoiceEditorScreen']

    def notifyBankChange(self, syx):
        '''Called when a new bank has been loaded'''
        self.voiceIsUpToDate=False
        for i in range(32):
            self.editBuffer.loadFromSyx(syx.getVoice(i))
            self.programBank.saveEditBuffer(i, self.editBuffer)

    def notifyProgramChange(self, program):
        '''Called at program change (invalidates edit buffer)'''
        self.voiceIsUpToDate=False
        self.currProgram=program

    def prepareEditMode(self):
        '''
        Set up the UI and Voice Edit Buffer on the MIDI device for Edit Mode
        '''
        if not self.voiceIsUpToDate:
            if self.programBank.getProgram(self.currProgram) is None:
                self.initVoiceEditor()
            else:
                self._loadCurrProgram()
            self.voiceIsUpToDate=True

    def sendSaveBufferMidi_(self, programNumber):
        SAVE_BUFFER_NRPN=7*128 + 3
        if self.midiOut:
            program14bit=programNumber*128
            self.midiOut.sendParameter(self.baseChannel,
                                       SAVE_BUFFER_NRPN,
                                       program14bit)

    def setOpEnabled_(self, newValues):
        oldValues=[]
        for n in range(6):
            paramName=f"Op{n+1} Operator Enable"
            old=self.editBuffer.getVoiceParameter(paramName)
            oldValues.append(old)
            if old!=newValues[n]:
                # update Operator Enable
                self.editBuffer.setVoiceParameter(paramName, newValues[n])
                self.editBuffer.sendVoiceParameter(paramName,
                                                   self.midiOut,
                                                   self.baseChannel)
        return oldValues # return old values so that they can be restored

    def saveVoice(self):
        '''
        saves voice-editor buffer

        updates program bank, including the persistent storage,
        sends SAVE_BUFFER <program#> nrpn to midi device
        and notifies the voice-select controller about updated voice
        '''

        # Operator Enable should not be part of saved voice
        # temporarily enable any disabled operator
        opEnabledOld=self.setOpEnabled_((1, 1, 1, 1, 1, 1))

        self.programBank.saveEditBuffer(self.currProgram, self.editBuffer)
        self.sendSaveBufferMidi_(self.currProgram)

        # restore disabled operators
        self.setOpEnabled_(opEnabledOld)
        
        # notify voice select controller
        self.voiceSelectController.notifyBufferStored(self.currProgram)

    def initVoiceEditor(self, updateUI=True):
        '''initialize UI and voice editor'''
        INIT_BUFFER=7*128 + 1

        self.editBuffer.setInitialVoice()
        if self.midiOut is not None:
            self.midiOut.sendParameter(self.baseChannel, INIT_BUFFER, 0)
        if updateUI:
            self._updateUI()

    def _loadCurrProgram(self):
        '''load a program into the edit buffer and update UI'''
        LOAD_BUFFER=7*128 + 2

        pgm=self.programBank.getProgram(self.currProgram)
        self.editBuffer.setVoiceParametersUnchecked(pgm)
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
        value=self._getVoiceParameter(paramName)
        self._updateUIField(paramName, value)

    def _updateUI(self):
        '''Updates the UI before switching to the VoiceEditScreen'''
        old=self.disableParameterUpdates
        self.disableParameterUpdates=True
        self.voiceEditorScreen.setVoiceParameter("Voice Number", str(self.currProgram+1))
        for (name, value) in self.editBuffer.getAllVoiceParameters():
            self._updateUIField(name, value)
        self.disableParameterUpdates=old

    def _getVoiceParameter(self, paramName):
        '''
        get voice parameter (incl. voice number, not in the buffer)

        note that Algorithm and Voice Number are base-zero, which
        mean that the minium value is zero (not one)
        '''
        return (self.editBuffer.getVoiceParameter(paramName)
                if paramName!="Voice Number" else self.currProgram)

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
            if paramValue!="": self.currProgram=paramValue
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

    def importProgram_(self, p):
        # start from initial Voice
        self.initVoiceEditor(updateUI=False)
        # apply parameters to editBuffer according to saved programs
        voiceMap=self.programBank.loadVoiceParameters(p)
        for (paramName, value) in voiceMap.items():
            self.editBuffer.setVoiceParameter(paramName, value)
            if self.midiOut is not None and paramName!="Voice Name":
                self.editBuffer.sendVoiceParameter(paramName,
                                                   self.midiOut,
                                                   self.baseChannel)
        # commit editBuffer to program bank: on the Daisy and on host
        self.programBank.setProgram(p, self.editBuffer.getVoiceParameters())
        self.sendSaveBufferMidi_(p) # Save the modified Edit Buffer->program p

    def importPrograms(self):
        '''load program from persistent storage and send to Daisy'''
        for p in self.programBank.getProgramDirectory():
            self.importProgram_(p)

def _isBaseOne(paramName):
    # Some integer parameters 0..N-1 are represented as 1..N in the UI
    return (paramName=="Voice Number" or paramName=="Algorithm")
