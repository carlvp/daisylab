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
