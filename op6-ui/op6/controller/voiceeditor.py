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

import os
import os.path

NUM_PROGRAMS=32
OP6_BANKS_DIR='op6-banks'

class VoiceEditorController:
    '''
    The VoiceController manages the UI of the VoiceEditorScreen
    and mediates user interaction and operations in the voice editor
    '''

    def __init__(self, editBuffer):
        self.editBuffer=editBuffer
        self.programBank=[None]*NUM_PROGRAMS
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

    def saveVoice(self):
        '''
        saves voice-editor buffer

        updates program bank
        sends SAVE_BUFFER <program#> nrpn to midi device
        notifies the voice-select controller about updated voice
        '''
        SAVE_BUFFER=7*128 + 3
        self.programBank[self.currProgram]=self.editBuffer.getVoiceParameters()
        if self.midiOut:
            program14bit=self.currProgram*128
            self.midiOut.sendParameter(self.baseChannel,
                                       SAVE_BUFFER,
                                       program14bit)

        # notify voice select controller
        name=self.editBuffer.getVoiceParameter("Voice Name")
        self.voiceSelectController.notifyBufferStored(self.currProgram, name)

        # print parameters to file
        bank=chr(65+self.currProgram//32) # bank A, B, C,...
        prog=1+(self.currProgram & 31)    # program 1, 2,...,32
        directory=os.path.join(OP6_BANKS_DIR, bank)

        def mkdir_p(path):
            if not os.path.exists(path):
                (head, tail)=os.path.split(path)
                if head!='':
                    mkdir_p(head)
                os.mkdir(path)

        mkdir_p(directory)
        filename=os.path.join(directory, f'{prog:02d}.json')
        with open(filename, 'w') as file:
            file.write('{\n')
            file.write(f'  "Voice Name": "{name}"')
            items=self.editBuffer.getAllVoiceParameters(skipInitialValue=True)
            for (param, value) in items:
                if param!="Voice Name":
                    file.write(f',\n  "{param}": {value}')
            file.write('\n}\n')

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

def _isBaseOne(paramName):
    # Some integer parameters 0..N-1 are represented as 1..N in the UI
    return (paramName=="Voice Number" or paramName=="Algorithm")
