PARAMETERS_PER_OPERATOR=128

# interface (MainController):
# registerControllerObjects()
# setView()
# updateUI()
#
# interface (View)
#
# interface (ProgramChangeListener)
# notifyProgramChange()

class VoiceEditorController:
    '''
    The VoiceController manages the UI of the VoiceEditorScreen
    and mediates user interaction and operations in the voice editor
    '''

    def __init__(self, editBuffer):
        self.editBuffer=editBuffer
        self.voiceEditorScreen=None
        self.screenIsUpToDate=False
        self.currSyx=None
        self.currProgram=0

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

    def updateUI(self):
        '''Updates the UI before switching to the VoiceEditScreen'''
        if not self.screenIsUpToDate:
            # Update editBuffer
            if self.currSyx is None:
                self.editBuffer.setInitialVoice()
            else:
                self.editBuffer.loadFromSyx(self.currSyx.getVoice(self.currProgram))
            # Update VoiceEditScreen from editBuffer
            self.voiceEditorScreen.setVoiceParameter("Voice Number", str(self.currProgram+1))
            for (name, value) in self.editBuffer.getAllVoiceParameters():
               self.voiceEditorScreen.setVoiceParameter(name, str(value))

        

