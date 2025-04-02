import tkinter

# interface (MainView):
# * registerModules()
# * resolveModules()

class PerformanceScreen(tkinter.Frame):
    '''
    The UI of the voice-edit mode
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)

        self.controller=None
        tkinter.Label(self,
                      text="This label is in the Performance Screen",
                      anchor=tkinter.N).grid(row=0, column=0)

    def registerModules(self, modules):
        '''adds this view object to the module dictionary.'''
        modules['PerformanceScreen']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.controller=modules['PerformanceController']
