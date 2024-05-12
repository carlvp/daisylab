import tkinter

class VoiceEditorScreen(tkinter.Frame):
    '''
    The UI of the voice-edit mode
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)

        self.controller=None
        tkinter.Label(self,
                      text="TODO: VoiceEditor Screen",
                      anchor=tkinter.N).grid(row=0, column=0)

    def registerViewObjects(self, views):
        '''adds view objects to the dictionary, views.'''
        views['VoiceEditorScreen']=self

    def setControllers(self, controllers):
        self.controller=controllers['VoiceEditorController']
