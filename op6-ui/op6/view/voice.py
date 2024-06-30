import tkinter
from tkinter import ttk
from . import colorscheme

# interface (MainView):
# * registerViewObjects()
# * setControllers()
#
# interface (PerformanceController):
# * selectVoice()

BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
LIGHT_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_HIGHLIGHTED

class VoiceEditorScreen(tkinter.Frame):
    '''
    The UI of the voice-edit mode
    * Controls the mode (performance/edit/compare) mode
    * Allows voice parameters to be edited
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)

        self.controller=None
        self.vars={}
        self._makeVoiceParamHeading(0)
        self._makeVoiceParamRow(1)
        self._makeOpParamHeading1(2)
        self._makeOpParamHeading2(3)
        for r in range(6):
            self._makeOpParamRow(6-r, 4+r)
        self._makeLfoParamHeading(10)
        self._makeLfoParamRow(11)

    def registerViewObjects(self, views):
        '''adds view objects to the dictionary, views.'''
        views['VoiceEditorScreen']=self

    def setControllers(self, controllers):
        voiceEditorController=controllers['VoiceEditorController']
        self.controller=voiceEditorController

    def _makeVoiceParamHeading(self, row):
        '''Create the headings of the Voice Params and Pitch EG'''
        self._makeLabel("Alg", row, 1)
        self._makeLabel("Fb", row, 2)
        self._makeLabel("Sync", row, 3)
        self._makeLabel("|", row, 4)
        self._makeLabel("PEG", row, 5)
        self._makeLabel("PM", row, 6)
        self._makeLabel("Vel", row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeLabel("T"+str(i+1), row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeLabel("L"+str(i), row, 14+i)
        self._makeLabel("|", row, 19)
        self._makeLabel("Rate", row, 20)

    def _makeVoiceParamRow(self, row):
        self.vars[1]=self._makeEntry(2, row, 1) # Algorithm
        self.vars[2]=self._makeEntry(1, row, 2) # Feedback
        self.vars[3]=self._makeCombobox(('', 'x'), 1, row, 3) # Sync
        self._makeLabel("|", row, 4)
        self.vars[5]=self._makeEntry(2, row, 5) # Pitch Env Mod
        self.vars[6]=self._makeEntry(2, row, 6)    # PM
        self.vars[7]=self._makeEntry(2, row, 7)    # Vel
        self._makeLabel("|", row, 8)
        for i in range(4):
            self.vars[8+i]=self._makeEntry(2, row, 9+i)    # EG Times 1-4
        self._makeLabel("|", row, 13)
        for i in range(5):
            self.vars[12+i]=self._makeEntry(2, row, 14+i)    # EG Levels 0-5
        self._makeLabel("|", row, 19)
        self.vars[17]=self._makeEntry(2, row, 20)  # Kbd Rate Scaling

    def _makeOpParamHeading1(self, row):
        '''Create the headings of the FM Operator rows (line #1)'''
        self._makeLabel("|", row, 4)
        self._makeLabel("|", row, 8)
        self._makeLabel("EG Times", row, 9, 4)
        self._makeLabel("|", row, 13)
        self._makeLabel("EG Levels", row, 14, 5)
        self._makeLabel("|", row, 19)
        self._makeLabel("Kbd", row, 20)
        self._makeLabel("Kbd level scaling", row, 21, 5)
        
    def _makeOpParamHeading2(self, row):
        '''Create the headings of the FM Operator rows (line #2)'''
        self._makeLabel("Op", row, 0)
        self._makeLabel("Freq", row, 1, 2)
        self._makeLabel("Mode", row, 3)
        self._makeLabel("|", row, 4)
        self._makeLabel("Out", row, 5)
        self._makeLabel("AM", row, 6)
        self._makeLabel("Vel", row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeLabel("T"+str(i+1), row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeLabel("L"+str(i), row, 14+i)
        self._makeLabel("|", row, 19)
        self._makeLabel("Rate", row, 20)
        self._makeLabel("Left", row, 21, 2)
        self._makeLabel("BP", row, 23)
        self._makeLabel("Right", row, 24, 2)
        
    def _makeOpParamRow(self, opNumber, row):
        self._makeLabel(str(opNumber), row, 0)     # Op
        self.vars[1]=self._makeEntry(6, row, 1, 2) # Freq
        self.vars[3]=self._makeCombobox(('x', 'Hz'), 2, row, 3)
        self._makeLabel("|", row, 4)
        self.vars[5]=self._makeEntry(2, row, 5)    # Out
        self.vars[6]=self._makeEntry(2, row, 6)    # AM
        self.vars[7]=self._makeEntry(2, row, 7)    # Vel
        self._makeLabel("|", row, 8)
        for i in range(4):
            self.vars[8+i]=self._makeEntry(2, row, 9+i)    # EG Times 1-4
        self._makeLabel("|", row, 13)
        for i in range(5):
            self.vars[12+i]=self._makeEntry(2, row, 14+i)    # EG Levels 0-5
        self._makeLabel("|", row, 19)
        self.vars[17]=self._makeEntry(2, row, 20)  # Kbd Rate Scaling
        self.vars[18]=self._makeEntry(3, row, 21)  # Left Depth
        self.vars[19]=self._makeCombobox(('Lin', 'Exp'), 3, row, 22)
        self.vars[20]=self._makeEntry(3, row, 23)  # Breakpoint
        self.vars[21]=self._makeEntry(3, row, 24)  # Right Depth
        self.vars[22]=self._makeCombobox(('Lin', 'Exp'), 3, row, 25)

    def _makeLfoParamHeading(self, row):
        '''Create the headings of the Voice Params and Pitch EG'''
        self._makeLabel("LFO Sp", row, 0, 2)
        self._makeLabel("Del", row, 2)
        self._makeLabel("Sync", row, 3)
        self._makeLabel("Wave", row, 5, 3)
        self._makeLabel("PMD", row, 9, 2)
        self._makeLabel("AMD", row, 11, 2)
        self._makeLabel("|", row, 13)
        self._makeLabel("|", row, 19)

    def _makeLfoParamRow(self, row):
        self.vars[1]=self._makeEntry(2, row, 1) # Speed
        self.vars[2]=self._makeEntry(2, row, 2) # Delay
        self.vars[3]=self._makeCombobox(('', 'x'), 1, row, 3) # Sync
        self.vars[5]=self._makeCombobox(('Triang', 'Saw Dn', 'Saw Up',
                                         'Sine',   'Square', 'S/H'),
                                        6, row, 5, 3) # Wave
        self.vars[8]=self._makeEntry(2, row, 9, 2)   # PMD
        self.vars[10]=self._makeEntry(2, row, 11, 2) # AMD
        self._makeLabel("|", row, 13)
        self._makeLabel("|", row, 19)

    def _makeLabel(self, text, row, column, columnspan=1):
        id=tkinter.Label(self,
                         text=text,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         anchor=tkinter.N)
        id.grid(row=row, column=column, columnspan=columnspan)
        
    def _makeEntry(self, width, row, column, columnspan=1):
        var=tkinter.StringVar(master=self, value='')
        id=tkinter.Entry(self,
                         textvariable=var,
                         justify=tkinter.RIGHT,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        return var

    def _makeCombobox(self, values, width, row, column, columnspan=1):
        var=tkinter.StringVar(master=self, value=values[0])
        id=RetroCombobox(self,
                         values,
                         textvariable=var,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        return var

class RetroCombobox(tkinter.Label):
    '''Retro-style multi-value entry widget'''
    def __init__(self, parent, values, **kwargs):
        tkinter.Label.__init__(self, parent, kwargs)
        self.values=values
        self.index=0
        
