import tkinter
from tkinter import ttk
from . import colorscheme

# interface (MainView):
# * registerViewObjects()
# * setControllers()
#
# interface (VoiceEditorController):
# * setVoiceParameter()

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
        self.parameterValue={}
        self._makeTopRow(0)
        self._makeVoiceParamHeading(1)
        self._makeVoiceParamRow(2)
        self._makeOpParamHeading1(3)
        self._makeOpParamHeading2(4)
        for r in range(6):
            self._makeOpParamRow(6-r, 5+r)
        self._makeLfoParamHeading(11)
        self._makeLfoParamRow(12)

    def registerViewObjects(self, views):
        '''adds view objects to the dictionary, views.'''
        views['VoiceEditorScreen']=self

    def setControllers(self, controllers):
        voiceEditorController=controllers['VoiceEditorController']
        self.controller=voiceEditorController

    def setVoiceParameter(self, paramName, paramValue):
        var=self.parameterValue[paramName]
        var.set(paramValue)

    def _makeTopRow(self, row):
        '''Creates the row with voice name and number'''
        self._makeLabel("Voice", row, 0, 2)
        self._makeEntry("Voice Number", 2, row, 2)
        voiceName = self._makeEntry("Voice Name", 24, row, 3, 9)
        voiceName.configure(justify=tkinter.LEFT)

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
        self._makeEntry("Algorithm", 2, row, 1)
        self._makeEntry("Feedback", 1, row, 2)
        self._makeCombobox("Oscillator Sync", ('', 'x'), 1, row, 3)
        self._makeLabel("|", row, 4)
        self._makeEntry("Pitch Envelope Depth", 2, row, 5)
        self._makeEntry("Pitch Modulation Sensitivity", 2, row, 6)
        self._makeEntry("Velocity Sensitivity", 2, row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeEntry("Pitch Envelope Time "+str(i+1),
                                           2, row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeEntry("Pitch Envelope Level "+str(i), 2, row, 14+i)
        self._makeLabel("|", row, 19)
        self._makeEntry("Keyboard Rate Scaling", 2, row, 20)

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
        prefix="Op"+str(opNumber)+" "
        self._makeLabel(str(opNumber), row, 0)
        self._makeEntry(prefix+"Frequency", 6, row, 1, 2)
        self._makeCombobox(prefix+"Frequency Mode", ('x', 'Hz'), 2, row, 3)
        self._makeLabel("|", row, 4)
        self._makeEntry(prefix+"Total Output Level", 2, row, 5)
        self._makeEntry(prefix+"Amplitude Modulation Sensitivity", 2, row, 6)
        self._makeEntry(prefix+"Velocity Sensitivity", 2, row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeEntry(prefix+"Envelope Time "+str(i+1), 2, row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeEntry(prefix+"Envelope Level "+str(i), 2, row, 14+i)
        self._makeLabel("|", row, 19)
        self._makeEntry(prefix+"Keyboard Rate Scaling", 2, row, 20)
        self._makeEntry(prefix+"Keyboard Level Scaling Left Depth", 3, row, 21)
        self._makeCombobox(prefix+"Keyboard Level Scaling Left Curve",
                           ('Lin', 'Exp'), 3, row, 22)
        self._makeEntry(prefix+"Keyboard Level Scaling Breakpoint", 3, row, 23)
        self._makeEntry(prefix+"Keyboard Level Scaling Right Depth", 3, row, 24)
        self._makeCombobox(prefix+"Keyboard Level Scaling Right Curve",
                           ('Lin', 'Exp'), 3, row, 25)

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
        self._makeEntry("LFO Speed", 2, row, 1) # Speed
        self._makeEntry("LFO Delay", 2, row, 2) # Delay
        self._makeCombobox("LFO Sync", ('', 'x'), 1, row, 3) # Sync
        self._makeCombobox("LFO Waveform", ('Triang', 'Saw Dn', 'Saw Up',
                                            'Sine',   'Square', 'S/H'),
                           6, row, 5, 3)
        self._makeEntry("LFO Initial Pitch Modulation Depth", 2, row, 9, 2)
        self._makeEntry("LFO Initial Amplitude Modulation Depth", 2, row, 11, 2)
        self._makeLabel("|", row, 13)
        self._makeLabel("|", row, 19)

    def _makeLabel(self, text, row, column, columnspan=1):
        id=tkinter.Label(self,
                         text=text,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         anchor=tkinter.N)
        id.grid(row=row, column=column, columnspan=columnspan)
        return id

    def _makeEntry(self, paramName, width, row, column, columnspan=1):
        var=tkinter.StringVar(master=self, value='')
        id=tkinter.Entry(self,
                         textvariable=var,
                         justify=tkinter.RIGHT,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        self.parameterValue[paramName]=var
        return id

    def _makeCombobox(self, paramName, values, width, row, column, columnspan=1):
        var=tkinter.StringVar(master=self, value=values[0])
        id=RetroCombobox(self,
                         values,
                         textvariable=var,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        self.parameterValue[paramName]=var
        return id

class RetroCombobox(tkinter.Label):
    '''Retro-style multi-value entry widget'''
    def __init__(self, parent, values, **kwargs):
        tkinter.Label.__init__(self, parent, kwargs)
        self.values=values
        self.index=0
        
