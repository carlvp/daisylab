PARAMETERS_PER_OPERATOR=128

class VoiceEditorController:
    '''
    The VoiceController manages the UI of the VoiceEditorScreen
    and mediates user interaction and operations in the voice editor
    '''

    def __init__(self):
        self.voiceEditorScreen=None

    def registerControllerObjects(self, controllers):
        '''adds controller objects to the dictionary, controllers.'''
        controllers['VoiceEditorController']=self

    def setViews(self, views):
        '''connects to relevant views in the dictionary, views'''
        self.voiceEditorScreen=views['VoiceEditorScreen']

    def setVoiceParameter(self, paramNumber, value):
        '''set given voice parameter to value'''
        pass

    def getVoiceParameterNumber(self, parameterName):
        '''get parameter number of named parameter'''
        return 0

    def getOperatorBaseNumber(self, opNumber):
        '''get the base parameter number (first parameter) of given operator'''
        return (6-opNumber)*PARAMETERS_PER_OPERATOR

