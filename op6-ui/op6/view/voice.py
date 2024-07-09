import tkinter
from tkinter import ttk
from . import colorscheme
from .resources import getPhotoImage

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
        self.algorithmDisplay=None
        # register Tk validation functions
        self.validateWidth=self.register(_onValidateWidth)
        self.validateInt=self.register(_onValidateInt)
        self.validateFp=self.register(_onValidateFp)
        # create layout
        self._makeAlgorithmLegend(0)
        self._makeTopRow(1)
        self._makeDisplayRow(2)
        self._makeVoiceParamHeading(3)
        self._makeVoiceParamRow(4)
        self._makeOpParamHeading1(5)
        self._makeOpParamHeading2(6)
        for r in range(6):
            self._makeOpParamRow(6-r, 7+r)
        self._makeLfoParamHeading(13)
        self._makeLfoParamRow(14)

    def registerViewObjects(self, views):
        '''adds view objects to the dictionary, views.'''
        views['VoiceEditorScreen']=self

    def setControllers(self, controllers):
        voiceEditorController=controllers['VoiceEditorController']
        self.controller=voiceEditorController

    def setVoiceParameter(self, paramName, paramValue):
        var=self.parameterValue[paramName]
        var.set(paramValue)

    def _onVoiceParamChanged(self, paramName, *_):
        var=self.parameterValue[paramName]
        paramValue=var.get()
        self.controller.updateVoiceParameter(paramName, paramValue)

    def _makeAlgorithmLegend(self, row):
        '''Creates the legend (image) showing all algorithms'''
        self._makeImage('algorithms.png', row, 0, columnspan=27)
        
    def _makeTopRow(self, row):
        '''Creates the row with voice name and number'''
        self._makeLabel("Voice", row, 0, 2)
        self._makeIntEntry("Voice Number", 2, row, 2, maxValue=32)
        voiceName = self._makeStringEntry("Voice Name", 24, row, 3, 9)

    def _makeDisplayRow(self, row):
        '''Creates the row with displays: algorithm and envelope'''
        self.algorithmDisplay=self._makeImage('algorithm1.png', row, 0, columnspan=7)

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
        self._makeIntEntry("Algorithm", 2, row, 1, maxValue=32)
        self._makeIntEntry("Feedback", 1, row, 2)
        self._makeCombobox("Oscillator Sync", ('', 'x'), 1, row, 3)
        self._makeLabel("|", row, 4)
        self._makeIntEntry("Pitch Envelope Depth", 2, row, 5)
        self._makeIntEntry("Pitch Modulation Sensitivity", 2, row, 6)
        self._makeIntEntry("Velocity Sensitivity", 2, row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeIntEntry("Pitch Envelope Time "+str(i+1),
                                           2, row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeIntEntry("Pitch Envelope Level "+str(i), 3, row, 14+i,
                               minValue=-99, maxValue=99)
        self._makeLabel("|", row, 19)
        self._makeIntEntry("Keyboard Rate Scaling", 2, row, 20)

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
        self._makeFpEntry(prefix+"Frequency", 6, row, 1, 2)
        self._makeCombobox(prefix+"Frequency Mode", ('x', 'Hz'), 2, row, 3)
        self._makeLabel("|", row, 4)
        self._makeIntEntry(prefix+"Total Output Level", 2, row, 5)
        self._makeIntEntry(prefix+"Amplitude Modulation Sensitivity", 2, row, 6)
        self._makeIntEntry(prefix+"Velocity Sensitivity", 2, row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeIntEntry(prefix+"Envelope Time "+str(i+1), 2, row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeIntEntry(prefix+"Envelope Level "+str(i), 2, row, 14+i)
        self._makeLabel("|", row, 19)
        self._makeIntEntry(prefix+"Keyboard Rate Scaling", 2, row, 20)
        self._makeIntEntry(prefix+"Keyboard Level Scaling Left Depth", 3, row, 21,
                        minValue=-99, maxValue=99)
        self._makeCombobox(prefix+"Keyboard Level Scaling Left Curve",
                           ('Lin', 'Exp'), 3, row, 22)
        self._makeIntEntry(prefix+"Keyboard Level Scaling Breakpoint", 3, row, 23)
        self._makeIntEntry(prefix+"Keyboard Level Scaling Right Depth", 3, row, 24,
                        minValue=-99, maxValue=99)
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
        self._makeIntEntry("LFO Speed", 2, row, 1) # Speed
        self._makeIntEntry("LFO Delay", 2, row, 2) # Delay
        self._makeCombobox("LFO Sync", ('', 'x'), 1, row, 3) # Sync
        self._makeCombobox("LFO Waveform", ('TRIANG', 'SAW DN', 'SAW UP',
                                            'SINE',   'SQUARE', 'S/HOLD'),
                           6, row, 5, 3)
        self._makeIntEntry("LFO Initial Pitch Modulation Depth", 2, row, 9, 2)
        self._makeIntEntry("LFO Initial Amplitude Modulation Depth", 2, row, 11, 2)
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

    def _makeIntEntry(self, paramName, width, row, column,
                   columnspan=1, minValue=0, maxValue=None):
        if maxValue is None:
            maxValue=10**width - 1
        vcmd=(self.validateInt, '%P', width, minValue, maxValue)

        var=tkinter.StringVar(master=self, value='', name=paramName)
        var.trace_add("write", self._onVoiceParamChanged)
        id=tkinter.Entry(self,
                         textvariable=var,
                         validate="key",
                         validatecommand=vcmd,
                         justify=tkinter.RIGHT,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        _setRetroEntryStyle(id)
        self.parameterValue[paramName]=var
        return id

    def _makeBase1Entry(self, paramName, width, row, column,
                        columnspan=1, maxValue=None):
        '''Formats a value in the range 0..N-1 as 1..N'''
        id=self._makeIntEntry(paramName, width, row, column,
                           columnspan, 0, maxValue)
        var=self.parameterValue[paramName]
        self.parameterValue[paramName]=Base1Formatter(var)
        return id

    def _makeFpEntry(self, paramName, width, row, column,
                     columnspan=1, maxValue=9999.9):
        '''Formats a frequency (floating-point) entry'''
        vcmd=(self.validateFp, '%P', width, 0.0, maxValue)

        var=tkinter.StringVar(master=self, value='', name=paramName)
        var.trace_add("write", self._onVoiceParamChanged)
        id=tkinter.Entry(self,
                         textvariable=var,
                         validate="key",
                         validatecommand=vcmd,
                         justify=tkinter.RIGHT,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        _setRetroEntryStyle(id)
        self.parameterValue[paramName]=FpFormatter(var)
        return id

    def _makeStringEntry(self, paramName, width, row, column, columnspan=1):
        vcmd=(self.validateWidth, '%P', width)
        var=tkinter.StringVar(master=self, value='', name=paramName)
        var.trace_add("write", self._onVoiceParamChanged)
        id=tkinter.Entry(self,
                         textvariable=var,
                         validate="key",
                         validatecommand=vcmd,
                         justify=tkinter.LEFT,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        _setRetroEntryStyle(id)
        self.parameterValue[paramName]=var
        return id

    def _makeCombobox(self, paramName, values, width, row, column, columnspan=1):
        var=tkinter.StringVar(master=self, value=values[0])
        id=tkinter.Label(self,
                         textvariable=var,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        self.parameterValue[paramName]=ComboboxFormatter(var, values)
        return id

    def _makeImage(self, name, row, column, rowspan=1, columnspan=1):
        img=getPhotoImage(name)
        id=tkinter.Label(self,
                         image=img,
                         anchor=tkinter.N)
        id.grid(row=row, column=column, rowspan=rowspan, columnspan=columnspan)
        _setRetroImageStyle(id)
        return id

class FpFormatter:
    '''Formats the frequency field which is floating point'''
    def __init__(self, var):
        self.var=var

    def set(self, value):
        freq=float(value)
        # frequencies < 100 Hz and ratios are formatted with three decimals
        value=(f'{freq:.3f}' if freq<99.9995 else
               f'{freq:.2f}' if freq<999.995 else
               f'{freq:.1f}')
        self.var.set(value)

    def get(self):
        return self.var.get()

class ComboboxFormatter:
    '''Formats a 0..N-1 as the values[i] strings'''
    def __init__(self, var, values):
        self.var=var
        self.values=values

    def set(self, value):
        self.var.set(self.values[int(value)])

    def get(self):
        return self.values.find(self.var.get())

def _onValidateWidth(newValue, width):
    '''validates the width of an entry (i.e. that it's not too long)'''
    return len(newValue)<=int(width)

def _onValidateInt(newValue, width, minValue, maxValue):
    '''validates a numeric (int) Entry, while it's being edited

    final saying when the update is sent to controller so it's useful to
    not to be too picky here (or editing becomes cumbersome)'''
    length=len(newValue)
    if (length>int(width)):
        return False
    elif length==0 or (newValue=="-" and int(minValue)<0):
        return True
    try:
        newValue=int(newValue)
        return int(minValue)<=newValue<=int(maxValue)
    except ValueError:
        return False

def _onValidateFp(newValue, width, minValue, maxValue):
    '''validates a numeric (float) Entry, while it's being edited

    final saying when the update is sent to controller so it's useful to
    not to be too picky here (or editing becomes cumbersome)'''
    length=len(newValue)
    if (length>int(width)):
        return False
    elif length==0 or newValue==".":
        return True
    try:
        newValue=float(newValue)
        return float(minValue)<=newValue<=float(maxValue)
    except ValueError:
        return False

class RetroCombobox(tkinter.Label):
    '''Retro-style multi-value entry widget'''
    def __init__(self, parent, values, **kwargs):
        tkinter.Label.__init__(self, parent, kwargs)
        self.values=values
        self.index=0

def _setRetroEntryStyle(widget):
    widget.config(foreground=FOREGROUND_COLOR,
                  background=BACKGROUND_COLOR,
                  borderwidth='0',
                  highlightthickness='0',
                  selectborderwidth='0',
                  selectbackground=FOREGROUND_COLOR,
                  selectforeground=BACKGROUND_COLOR,
                  insertbackground=FOREGROUND_COLOR)

def _setRetroImageStyle(widget):
    widget.config(foreground=FOREGROUND_COLOR,
                  background=BACKGROUND_COLOR,
                  borderwidth='0',
                  highlightthickness='0')
