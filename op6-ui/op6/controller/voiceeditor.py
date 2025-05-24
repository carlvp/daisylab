# interface (MainController):
# registerModules()
# resolveModules()
# setHasActiveScreen()
# setMidiOut()
# setDisplay()
# setCurrentParameter()
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
        self.display=None
        self.displayLine1=None
        self.displayLine2=None
        self.currText=None
        self.currParam=None
        self.currParamValue=0
        self.currAlgorithm=0
        self.currOp=None
        self.enabledOps="123456"
        self.setCurrentParameter("Algorithm")

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
        # update display
        if self.currParam is not None:
            self.currParamValue=self._getVoiceParameter(self.currParam)
        self.enabledOps=self._getEnabledOps()
        self.currAlgorithm=self._getVoiceParameter("Algorithm")
        self._updateDisplay()

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
        # update display
        changed=False
        if paramName==self.currParam:
            if paramValue!=self.currParamValue:
                self.currParamValue=self._getVoiceParameter(self.currParam)
                changed=True
        if paramName=="Algorithm" and paramValue!="":
            self.currAlgorithm=paramValue
            changed=True
        if paramName.endswith("Operator Enable"):
            self.enabledOps=self._getEnabledOps()
            changed=True
        if changed:
            self._updateDisplay()

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

    def syncToEditBuffer_(self, pgm):
        # start from initial Voice (clear edit buffer on host and device)
        self.initVoiceEditor(updateUI=False)
        # load program into edit buffer on host
        if pgm is not None:
            self.editBuffer.setVoiceParametersUnchecked(pgm)
        # send to device
        self.editBuffer.sendAllVoiceParameters(self.midiOut, self.baseChannel,
                                               skipInitialValue=True)

    def syncProgram_(self, index):
        pgm=self.programBank.getProgram(index)
        self.syncToEditBuffer_(pgm)
        self.sendSaveBufferMidi_(index)

    def syncProgramsOnConnect(self, numPrograms):
        '''send programs to Daisy'''
        savedBuffer=self.editBuffer.asProgram()
        for index in range(numPrograms):
            self.syncProgram_(index)
        # restore the saved buffer on host and device
        self.syncToEditBuffer_(savedBuffer)

    def setDisplay(self, display):
        '''sets the active status of the display
        
        the active controller owns the screen and the display.
        when the controller is not active, display is None
        '''
        self.display=display
        self._updateDisplay()

    def setCurrentParameter(self, paramName):
        '''selects the parameter to be displayed and affected by +/-'''
        self.currParam=paramName
        if paramName!="Save" and paramName!="Init":
            self.currParamValue=self._getVoiceParameter(self.currParam)
        # split paramName in operator numer (or None) and rest of the name
        self.currOp=int(paramName[2]) if paramName.startswith("Op") else None
        key=paramName[4:] if paramName.startswith("Op") else paramName
        # Look up display formatting details
        self.currText=VoiceEditorController.displayTexts.get(key, key)
        self.displayLine1=VoiceEditorController.displayLine1.get(key, VoiceEditorController._displayLine1AlgOps)
        self.displayLine2=VoiceEditorController.displayLine2.get(key, VoiceEditorController._displayLine2TextNN)
        self._updateDisplay()

    def _updateDisplay(self):
        if self.display is not None:
            l1=self.displayLine1(self)
            l2=self.displayLine2(self)
            self.display.update(l1, l2)

    def _displayLine1AlgOps(self):
        alg=self.currAlgorithm+1
        return (f"ALG{alg:02} {self.enabledOps}" if self.currOp is None else
                f"ALG{alg:02} {self.enabledOps} OP{self.currOp}")

    def _getEnabledOps(self):
        DASH=45
        s=bytearray(b"123456")
        for n in range(6):
            paramName=f"Op{n+1} Operator Enable"
            if self.editBuffer.getVoiceParameter(paramName)==0:
                s[n]=DASH
        return s.decode()

    def _displayLine1Voice(self, suffix=""):
        n=self.currProgram+1
        bankLetter='A'
        voiceNumber=n
        return f"Voice {bankLetter}{voiceNumber:02}{suffix}"

    def _displayLine1VoiceName(self):
        return self._displayLine1Voice(" Name:")

    def _displayLine2TextNN(self):
        return f"{self.currText}={self.currParamValue:2}"

    def _displayLine2TextN(self):
        return f"{self.currText}={self.currParamValue}"

    def _displayLine2Text(self):
        return self.currText

    def _displayLine2Value(self):
        return self.currParamValue

    def _displayLine2TextOnOff(self):
        status="ON" if self.currParamValue!=0 else "OFF"
        return f"{self.currText} {status}"

    def _displayLine2FreqMode(self):
        return ("Fixed Freq. Mode" if self.currParamValue!=0 else
                "Freq. Ratio Mode")

    _klsCurve=("-Lin", "-Exp", "+Exp", "+Lin")

    def _displayLine2KlsCurve(self):
        curve=VoiceEditorController._klsCurve[self.currParamValue]
        return f"{self.currText}={curve}"

    _lfoWaveforms=(
        "TRIANGL",
        "SINE",
        "SAW UP",
        "SAW DN",
        "SQUARE",
        "S&H",
    )

    def _displayLine2LfoWave(self):
        n=self.currParamValue
        return f"LFO Wave={VoiceEditorController._lfoWaveforms[n]}"

    # Override default text: parameter name
    displayTexts={
        "Voice Number":                           "Set Voice Number",
        "Save":                                   "Save?       [OK]",
        "Init":                                   "Initialize? [OK]",
        "Algorithm":                              "Select Algorithm",
        "Pitch Modulation Sensitivity":           "PM Sensitivity",
        "Pitch Envelope Time 1":                  "Pitch Env. T1",
        "Pitch Envelope Time 2":                  "Pitch Env. T2",
        "Pitch Envelope Time 3":                  "Pitch Env. T3",
        "Pitch Envelope Time 4":                  "Pitch Env. T4",
        "Pitch Envelope Level 0":                 "Pitch Env. L0",
        "Pitch Envelope Level 1":                 "Pitch Env. L1",
        "Pitch Envelope Level 2":                 "Pitch Env. L2",
        "Pitch Envelope Level 3":                 "Pitch Env. L3",
        "Pitch Envelope Level 4":                 "Pitch Env. L4",
        "Operator Enable":                        "Operator",
        "Total Output Level":                     "Output Level",
        "Amplitude Modulation Sensitivity":       "AM Sensitivity",
        "Velocity Sensitivity":                   "Velocity Sens.",
        "Envelope Time 1":                        "Envelope T1",
        "Envelope Time 2":                        "Envelope T2",
        "Envelope Time 3":                        "Envelope T3",
        "Envelope Time 4":                        "Envelope T4",
        "Envelope Level 0":                       "Envelope L0",
        "Envelope Level 1":                       "Envelope L1",
        "Envelope Level 2":                       "Envelope L2",
        "Envelope Level 3":                       "Envelope L3",
        "Envelope Level 4":                       "Envelope L4",
        "Keyboard Rate Scaling":                  "Kbd Rate Scale",
        "Keyboard Level Scaling Left Depth":      "Left KLS",
        "Keyboard Level Scaling Left Curve":      "Left Curve",
        "Keyboard Level Scaling Breakpoint":      "Breakpoint",
        "Keyboard Level Scaling Right Depth":     "Right KLS",
        "Keyboard Level Scaling Right Curve":     "Right Curve",
        "LFO Initial Pitch Modulation Depth":     "LFO PM Depth",
        "LFO Initial Amplitude Modulation Depth": "LFO AM Depth",
    }

    # Override default formatter: _displayLine1AlgOps
    displayLine1={
        "Voice Number":                           _displayLine1Voice,
        "Voice Name":                             _displayLine1VoiceName,
        "Save":                                   _displayLine1Voice,
        "Init":                                   _displayLine1Voice,
    }

    # Override default formatter: _displayLine2TextNN
    displayLine2={
        "Voice Number":                           _displayLine2Text,
        "Voice Name":                             _displayLine2Value,
        "Save":                                   _displayLine2Text,
        "Init":                                   _displayLine2Text,
        "Algorithm":                              _displayLine2Text,
        "Feedback":                               _displayLine2TextN,
        "Pitch Modulation Sensitivity":           _displayLine2TextN,
        "Operator Enable":                        _displayLine2TextOnOff,
        "Frequency Mode":                         _displayLine2FreqMode,
        "Amplitude Modulation Sensitivity":       _displayLine2TextN,
        "Velocity Sensitivity":                   _displayLine2TextN,
        "Keyboard Rate Scaling":                  _displayLine2TextN,
        "Keyboard Level Scaling Left Curve":      _displayLine2KlsCurve,
        "Keyboard Level Scaling Right Curve":     _displayLine2KlsCurve,
        "LFO Waveform":                           _displayLine2LfoWave,
    }

def _isBaseOne(paramName):
    # Some integer parameters 0..N-1 are represented as 1..N in the UI
    return (paramName=="Voice Number" or paramName=="Algorithm")
