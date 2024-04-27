import tkinter

class PerformanceScreen(tkinter.Frame):
    '''
    The UI of the performance mode:
    * Channel settings (CC), such as volume and pan can be modified
    * MIDI Program change (i.e. voice) via a menu
    * Program banks (SysEx files) can be loaded
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)

        tkinter.Label(self,
                      text="This label is in the Performance Screen",
                      anchor=tkinter.N).grid(row=0, column=0)
        
    def registerViewObjects(self, views):
        '''adds view objects to the dictionary, views.'''
        views['PerformanceScreen']=self

    def setControllers(self, controllers):
        self.controller=controllers['PerformanceController']
