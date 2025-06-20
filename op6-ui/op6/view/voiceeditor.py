import bisect
import tkinter
from tkinter import ttk
from . import colorscheme
from .resources import getPhotoImage

# interface (MainView):
# * registerModules()
# * resolveModules()
#
# interface (VoiceEditorController):
# * setVoiceParameter()

BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
LIGHT_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_HIGHLIGHTED
DARK_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_DARK

# Screen Layout
TOP_ROW=1
VOICE_PARAM_ROW=4
OP6_ROW=7
LFO_ROW=14

# The cursor snaps to the following rows
_CURSOR_ROWS=[
    TOP_ROW,   VOICE_PARAM_ROW, OP6_ROW,   OP6_ROW+1, OP6_ROW+2,
    OP6_ROW+3, OP6_ROW+4,       OP6_ROW+5, LFO_ROW
]

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
        self.envelopeDisplay=None
        self.kbdScalingDisplay=None
        self.saveButton=None
        self.cursorRow=0
        self.cursorCol=0
        self.currWidget=None
        # register Tk validation functions
        self.validateWidth=self.register(_onValidateWidth)
        self.validateInt=self.register(_onValidateInt)
        self.validateFp=self.register(_onValidateFp)
        # create layout
        self._makeAlgorithmLegend(0)
        self._makeTopRow(TOP_ROW)
        self._makeDisplayRow(TOP_ROW+1)
        self._makeVoiceParamHeading(VOICE_PARAM_ROW-1)
        self._makeVoiceParamRow(VOICE_PARAM_ROW)
        self._makeOpParamHeading1(OP6_ROW-2)
        self._makeOpParamHeading2(OP6_ROW-1)
        for r in range(6):
            self._makeOpParamRow(6-r, OP6_ROW+r)
        self._makeLfoParamHeading(LFO_ROW-1)
        self._makeLfoParamRow(LFO_ROW)
        # initialize displays
        self._connectEnvelopeVariables()
        self.currDisplayed=None
        # set initial focus on Algorithm Entry
        self._getWidget(VOICE_PARAM_ROW, 1).focus()

    def registerModules(self, modules):
        '''adds this view object to the module dictionary.'''
        modules['VoiceEditorScreen']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.controller=modules['VoiceEditorController']

    def setVoiceParameter(self, paramName, paramValue):
        var=self.parameterValue[paramName]
        var.set(paramValue)
        self._extraUpdateAction(paramName, paramValue)

    def _onVoiceParamChanged(self, paramName, *_):
        '''listens to parameter values changed via UI'''
        var=self.parameterValue[paramName]
        paramValue=var.get()

        self.controller.updateVoiceParameter(paramName, paramValue)
        self._extraUpdateAction(paramName, paramValue)

    def _focusOnRow(self, row):
        opNumber=(0             if row==VOICE_PARAM_ROW   else
                  OP6_ROW+6-row if OP6_ROW<=row<OP6_ROW+6 else -1) 
        self._displayOperator(opNumber)

    def _displayOperator(self, opNumber):
        '''-1=None, 0=pitch envelope, 1..6=operator envelope and kbd scaling'''
        if opNumber<1:
            leftDepth=None
            leftCurve=None
            rightDepth=None
            rightCurve=None
            self.currDisplayed="Pitch"
        else:
            self.currDisplayed="Op"+str(opNumber)
            prefix=self.currDisplayed+" Keyboard Level Scaling "
            leftDepth=self.parameterValue[prefix+"Left Depth"]
            leftCurve=self.parameterValue[prefix+"Left Curve"]
            rightDepth=self.parameterValue[prefix+"Right Depth"]
            rightCurve=self.parameterValue[prefix+"Right Curve"]
        self.kbdScalingDisplay.setVariables(leftDepth, leftCurve,
                                            rightDepth, rightCurve)
        self.kbdScalingDisplay.onParameterChanged()
        self.envelopeDisplay.setOperator(opNumber)

    def _extraUpdateAction(self, paramName, paramValue):
        '''some parameters have extra update actions, handled here'''
        if paramName=="Algorithm":
            # Update Algorithm Display
            self._updateAlgorithmDisplay(paramValue)
        elif (self.currDisplayed is not None and
              paramName.startswith(self.currDisplayed)):
            if paramName[4:12]=="Keyboard":
                # Update Keyboard-level Scaling Display
                # filter out Depth and Curve parameters
                # (not critical to filter, but doesn't hurt I suppose)
                if paramName.endswith("Depth") or paramName.endswith("Curve"):
                    self.kbdScalingDisplay.onParameterChanged()
            else:
                p=len(self.currDisplayed)+1
                if paramName[p:p+8]=="Envelope":
                    self.envelopeDisplay.onParameterChanged()
        

    def _makeAlgorithmLegend(self, row):
        '''Creates the legend (image) showing all algorithms'''
        self._makeImage('algorithms.png', row, 0, columnspan=26)

    def _makeTopRow(self, row):
        '''Creates the row with voice name and number'''
        self._makeLabel("Voice", row, 4, 2)
        self._makeIntEntry("Voice Number", 2, row, 6, maxValue=32)
        voiceName = self._makeStringEntry("Voice Name", 24, row, 7, columnspan=7)
        # buttons
        self.saveButton=tkinter.Button(self,
                                       text="Save",
                                       command=(lambda:
                                                self.controller.saveVoice()))
        self.saveButton.grid(row=row, column=15, columnspan=2, sticky=tkinter.S)
        self.saveButton.bind('<FocusIn>', self._focusInListener)
        button=tkinter.Button(self,
                              text="Init",
                              command=lambda: self.controller.initVoiceEditor())
        button.grid(row=row, column=17, columnspan=2, sticky=(tkinter.S, tkinter.E))
        button.bind('<FocusIn>', self._focusInListener)

    def _makeDisplayRow(self, row):
        '''Creates the row with displays: algorithm and envelope'''
        self.algorithmDisplay=self._makeImage('algorithm1.png', row, 0, columnspan=7)
        self.envelopeDisplay=EnvelopeDisplay(self)
        self.envelopeDisplay.grid(row=row, column=7, columnspan=13)
        self.kbdScalingDisplay=KeyboardDisplay(self)
        self.kbdScalingDisplay.grid(row=row, column=21, rowspan=3, columnspan=5)
        
    def _updateAlgorithmDisplay(self, number):
        '''Update the display to show the algorithm with the given number'''
        if number!="" and 1<=int(number)<=32:
            img=getPhotoImage("algorithm"+str(number)+".png")
            self.algorithmDisplay.config(image=img)

    def _makeVoiceParamHeading(self, row):
        '''Create the headings of the Voice Params and Pitch EG'''
        self._makeLabel("Alg", row, 1)
        self._makeLabel("Fb", row, 2)
        # self._makeLabel("Sync", row, 3)
        self._makeLabel("|", row, 4)
        # self._makeLabel("PEG", row, 5)
        self._makeLabel("PM", row, 6)
        # self._makeLabel("Vel", row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeLabel("T"+str(i+1), row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeLabel("L"+str(i), row, 14+i)
        self._makeLabel("|", row, 19)
        # self._makeLabel("Rate", row, 20)

    def _makeVoiceParamRow(self, row):
        self._makeIntEntry("Algorithm", 2, row, 1, maxValue=32)
        self._makeIntEntry("Feedback", 1, row, 2)
        # self._makeCombobox("Oscillator Sync", ('', 'x'), 1, row, 3)
        self._makeLabel("|", row, 4)
        # self._makeIntEntry("Pitch Envelope Depth", 2, row, 5)
        self._makeIntEntry("Pitch Modulation Sensitivity", 2, row, 6)
        # self._makeIntEntry("Velocity Sensitivity", 2, row, 7)
        self._makeLabel("|", row, 8)
        for i in range(4):
            self._makeIntEntry("Pitch Envelope Time "+str(i+1),
                                           2, row, 9+i)
        self._makeLabel("|", row, 13)
        for i in range(5):
            self._makeIntEntry("Pitch Envelope Level "+str(i), 3, row, 14+i,
                               minValue=-99, maxValue=99)
        self._makeLabel("|", row, 19)
        # self._makeIntEntry("Keyboard Rate Scaling", 2, row, 20)

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
        opStr=str(opNumber)
        prefix="Op"+opStr+" "
        self._makeToggle(prefix+"Operator Enable", "1", opStr, 1, row, 0)
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
        self._makeIntEntry(prefix+"Keyboard Level Scaling Left Depth",
                           2, row, 21)
        self._makeCombobox(prefix+"Keyboard Level Scaling Left Curve",
                           ('-Lin', '-Exp', '+Exp', '+Lin'), 4, row, 22)
        self._makeIntEntry(prefix+"Keyboard Level Scaling Breakpoint", 3, row, 23)
        self._makeIntEntry(prefix+"Keyboard Level Scaling Right Depth",
                           2, row, 24)
        self._makeCombobox(prefix+"Keyboard Level Scaling Right Curve",
                           ('-Lin', '-Exp', '+Exp', '+Lin'), 4, row, 25)

    def _makeLfoParamHeading(self, row):
        '''Create the headings of the Voice Params and Pitch EG'''
        self._makeLabel("LFO Sp", row, 0, 2)
        self._makeLabel("Del", row, 2)
        self._makeLabel("Wave", row, 5, 3)
        self._makeLabel("PMD", row, 9, 2)
        self._makeLabel("AMD", row, 11, 2)
        self._makeLabel("|", row, 13)

    def _makeLfoParamRow(self, row):
        self._makeIntEntry("LFO Speed", 2, row, 1)
        self._makeIntEntry("LFO Delay", 2, row, 2)
        self._makeCombobox("LFO Waveform", ('TRIANG', 'SAW DN', 'SAW UP',
                                            'SINE',   'SQUARE', 'S/HOLD'),
                           6, row, 5, 3)
        self._makeIntEntry("LFO Initial Pitch Modulation Depth", 2, row, 9, 2)
        self._makeIntEntry("LFO Initial Amplitude Modulation Depth", 2, row, 11, 2)
        self._makeLabel("|", row, 13)

    def _makeLabel(self, text, row, column, columnspan=1):
        id=tkinter.Label(self,
                         text=text,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         anchor=tkinter.N)
        id.grid(row=row, column=column, columnspan=columnspan)
        return id

    def _focusInListener(self, e, selectAll=False):
        widget=e.widget
        info=widget.grid_info()
        self.cursorRow=info["row"]
        self.cursorCol=info["column"]
        self._focusOnRow(self.cursorRow)
        self.currWidget=widget
        self.controller.setCurrentParameter(self._getParamName(widget))
        if selectAll:
            widget.select_range(0, tkinter.END)
        else:
            # clear any selection elsewhere
            self.selection_clear()

    def _getParamName(self, widget):
        # button Save/Init have no textvariables
        key="text" if type(widget)==tkinter.Button else "textvariable"
        return widget.cget(key)

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
        id.bind('<FocusIn>', lambda e: self._focusInListener(e, selectAll=True))
        id.bind('<FocusOut>', lambda _, name=paramName:
                self.controller.requestUIFieldUpdate(name))
        _setRetroEntryStyle(id)
        self.parameterValue[paramName]=var
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
        id.bind('<FocusIn>', lambda e: self._focusInListener(e, selectAll=True))
        id.bind('<FocusOut>', lambda _, name=paramName:
                self.controller.requestUIFieldUpdate(name))
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
        id.bind('<FocusIn>', lambda e: self._focusInListener(e, selectAll=True))
        id.bind('<FocusOut>', lambda _, name=paramName:
                self.controller.requestUIFieldUpdate(name))
        _setRetroEntryStyle(id)
        self.parameterValue[paramName]=var
        return id

    def _makeCombobox(self, paramName, values, width, row, column, columnspan=1):
        var=tkinter.StringVar(master=self, value=values[0], name=paramName)
        var.trace_add("write", self._onVoiceParamChanged)
        id=RetroCombobox(self, var, values, width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        cboxFormatter=ComboboxFormatter(var, values)
        self.parameterValue[paramName]=cboxFormatter
        id.bind('<FocusIn>', self._focusInListener)
        return id

    def _makeToggle(self, paramName, initialValue,
                    text, width, row, column, columnspan=1):
        var=tkinter.StringVar(master=self, value=str(initialValue),
                              name=paramName)
        id=RetroToggle(self, var, text=text, width=width)
        id.grid(row=row, column=column, columnspan=columnspan)
        self.parameterValue[paramName]=var
        var.trace_add("write",
                      lambda name, index, op, widget=id:
                          (widget.master._onVoiceParamChanged(name, index, op),
                           widget.update()))
        id.bind('<FocusIn>', self._focusInListener)
        return id

    def _makeImage(self, name, row, column, rowspan=1, columnspan=1):
        img=getPhotoImage(name)
        id=tkinter.Label(self,
                         image=img,
                         anchor=tkinter.N)
        id.grid(row=row, column=column, rowspan=rowspan, columnspan=columnspan)
        _setRetroImageStyle(id)
        return id

    def _connectEnvelopeVariables(self):
        for op in range(0,7):
            # 0 = pitch envelope, 1..6 for the operators
            prefix="Pitch" if op==0 else "Op"+str(op)
            self.envelopeDisplay.setTimeVar(op,
                [self.parameterValue[prefix+" Envelope Time "+str(i+1)]
                 for i in range(4)])
            self.envelopeDisplay.setLevelVar(op,
                [self.parameterValue[prefix+" Envelope Level "+str(i)]
                 for i in range(5)])

    def _setOpRowForeground(self, opNumber, color):
        widgets=self.grid_slaves(row=OP6_ROW+6-opNumber)
        for w in widgets:
            if type(w)!=tkinter.Label and type(w)!=RetroToggle:
                w.config(foreground=color)

    def _snap(L, x, dx):
        i=bisect.bisect_left(L, x)
        if i<len(L):
            y=L[i]
            if x!=y and dx==-1:
                # go to previous element (or clamp at first)
                y = L[i-1] if i>0 else L[0]
        else:
            # clamp at last element
            y=L[len(L)-1]
        return y

    def moveCursor(self, deltaRow, deltaCol):
        if deltaRow!=0:
            r=VoiceEditorScreen._snap(_CURSOR_ROWS, self.cursorRow+deltaRow, deltaRow)
            widget=self._getWidget(r, self.cursorCol)
            if widget is None:
                # adjust column
                c=self.cursorCol
                if r==TOP_ROW:
                    c=6 if c<6 else 17
                elif r==VOICE_PARAM_ROW:
                    c=(1 if c<1 else
                       2 if c<4 else
                       6 if c<8 else 18)
                elif r==LFO_ROW:
                    c=(1 if c<1 else
                       2 if c<4 else 12)
                widget=self._getWidget(r, c)
        else:
            widget=(self.currWidget.tk_focusNext() if deltaCol==+1 else
                    self.currWidget.tk_focusPrev())
        if widget.master==self:
            widget.focus()

    def _getWidget(self, row, column):
        widgets=self.grid_slaves(row=row, column=column)
        return widgets[0] if len(widgets)==1 else None

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
        return self.values.index(self.var.get())

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

class RetroCombobox(tkinter.Button):
    '''Retro-style multi-value entry widget'''
    def __init__(self, parent, var, values, **kwargs):
        tkinter.Button.__init__(self, parent, kwargs)
        self.config(foreground=FOREGROUND_COLOR,
                    background=BACKGROUND_COLOR,
                    activeforeground=LIGHT_FOREGROUND_COLOR,
                    activebackground=BACKGROUND_COLOR,
                    highlightcolor=FOREGROUND_COLOR,
                    highlightbackground=BACKGROUND_COLOR,
                    highlightthickness=1,
                    borderwidth=0,
                    padx=1,
                    pady=1,
                    textvariable=var,
                    command=self.buttonHandler)
        self.values=values
        self.var=var

    def cget(self, key):
        return str(self.var) if key=="textvariable" else tkinter.Button(self, key)

    def buttonHandler(self):
        if self.focus_get()!=self:
            self.focus_set()
        n=self.values.index(self.var.get())+1
        if n==len(self.values):
            n=0
        self.var.set(self.values[n])

class RetroToggle(tkinter.Button):
    '''Retro-style toggle widget'''
    def __init__(self, parent, var, **kwargs):
        tkinter.Button.__init__(self, parent, kwargs)
        self.config(highlightcolor=FOREGROUND_COLOR,
                    highlightbackground=BACKGROUND_COLOR,
                    highlightthickness=1,
                    borderwidth=0,
                    padx=1,
                    pady=1,
                    command=self._buttonHandler)
        self.var=var
        self._updateConfig(self._getValue())

    def cget(self, key):
        return str(self.var) if key=="textvariable" else tkinter.Button(self, key)

    def update(self):
        self._update(self._getValue())

    def _buttonHandler(self):
        if self.focus_get()!=self:
            self.focus_set()
        self._setValue(1-self._getValue())
        
    def _getValue(self):
        return int(self.var.get())
    
    def _setValue(self, newValue):
        self.var.set(str(newValue))
        self._update(newValue)

    def _updateConfig(self, value):
        # Reverse (black on green) when value=1
        (fg, bg, activeFg, activeBg)=(
            (BACKGROUND_COLOR,
             FOREGROUND_COLOR,
             BACKGROUND_COLOR,
             LIGHT_FOREGROUND_COLOR) if value==1 else
            (DARK_FOREGROUND_COLOR,
             BACKGROUND_COLOR,
             LIGHT_FOREGROUND_COLOR,
             BACKGROUND_COLOR))
        self.config(foreground=fg,
                    background=bg,
                    activeforeground=activeFg,
                    activebackground=activeBg)

    def _update(self, value):
        self._updateConfig(value)
        op=int(self['text'])
        color=FOREGROUND_COLOR if value==1 else DARK_FOREGROUND_COLOR
        self.master._setOpRowForeground(op, color)

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


ENVELOPE_HIDDEN=-1
PITCH_ENVELOPE=0
# Op# 1..6 for the other envelope curves

class EnvelopeDisplay(tkinter.Frame):
    '''The Display, which illustrates envelope parameters'''
    WIDTH=408
    HEIGHT=120
    ORIGINX=4
    ORIGINY=8

    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        self.canvas=tkinter.Canvas(self,
                                   width=EnvelopeDisplay.WIDTH,
                                   height=EnvelopeDisplay.HEIGHT,
                                   background=BACKGROUND_COLOR,
                                   borderwidth='0',
                                   highlightthickness='0')
        # draw background grid
        x0=EnvelopeDisplay.ORIGINX
        y0=EnvelopeDisplay.ORIGINY
        x1=x0+400
        y1=y0+100
        self.canvas.create_line(x0, y0, x0, y1, fill=DARK_FOREGROUND_COLOR)
        self.canvas.create_line(x1, y0, x1, y1, fill=DARK_FOREGROUND_COLOR)
        for y in range(y0, y0+101, 10):
            self.canvas.create_line(x0, y, x1, y, fill=DARK_FOREGROUND_COLOR)
        self.canvas.pack(fill=tkinter.NONE, expand=False)
        # variables (pitch envelope and op1-op6 envelope)
        self.levelVar=[None, None, None, None, None, None, None]
        self.timeVar=[None, None, None, None, None, None, None]
        # start w/o any curve displayed
        self.operator=-1
        self.coordinates=[x0, y1, x0+1, y0,   # attack
                          x0+2, y0, x0+3, y0, # decay 1 and 2
                          x1-1, y0, x1, y1]   # sustain and release
        self.ad1d2Curve=self.canvas.create_line(*self.coordinates[:8],
                                                fill=FOREGROUND_COLOR,
                                                width=2,
                                                state='hidden')
        self.sLine=self.canvas.create_line(*self.coordinates[6:10],
                                            fill=FOREGROUND_COLOR,
                                            width=2,
                                            dash=(3,5),
                                            state='hidden')
        self.rLine=self.canvas.create_line(*self.coordinates[8:],
                                            fill=FOREGROUND_COLOR,
                                            width=2,
                                            state='hidden')
    def _setCurveState(self, s):
        self.canvas.itemconfig(self.ad1d2Curve, state=s)
        self.canvas.itemconfig(self.sLine, state=s)
        self.canvas.itemconfig(self.rLine, state=s)

    def setOperator(self, index):
        if index!=self.operator:
            if 0<=index<=6:
                if self.operator==ENVELOPE_HIDDEN:
                    self._setCurveState('normal')
                self.operator=index
                self.onParameterChanged()
            else:
                self.operator=ENVELOPE_HIDDEN
                self._setCurveState('hidden')

    def setTimeVar(self, index, variables):
        '''set envelope time variables.

        index=0 for pitch envelope, 1..6 for the envelopes of the operators
        variables is a tuple of StringVar (t1, t2, t3, t4)
        '''
        self.timeVar[index]=variables

    def setLevelVar(self, index, variables):
        '''set envelope level variables.

        index=0 for pitch envelope, 1..6 for the envelopes of the operators
        variables is a tuple of StringVar (l0, l1, l2, l3, l4)
        '''
        self.levelVar[index]=variables

    def level2y(self, level):
        # operator=1..6 or 0 for pitch envelope
        (y0, scale) = (50, 0.5) if self.operator==PITCH_ENVELOPE else (100, 1)
        return EnvelopeDisplay.ORIGINY + y0 - level*scale

    def y2level(self, y):
        # operator=1..6 or 0 for pitch envelope
        (y0, scale, low) = (50, 2, -99) if self.operator==PITCH_ENVELOPE else (100, 1, 0)
        return max(low, min(99, (EnvelopeDisplay.ORIGINY + y0 - y)*scale))

    def _updateLevel(self, lIndex, coordIndex, changed):
        level=self.levelVar[self.operator][lIndex].get()
        if level!="" and level!="-":
            y=self.level2y(int(level))
            if self.coordinates[coordIndex] != y:
                self.coordinates[coordIndex]=y
                return True
        return changed

    def onParameterChanged(self):
        '''checks if the curve has changed, in which case it's updated'''
        if self.operator==ENVELOPE_HIDDEN:
            self._setCurveState('hidden')

        updateCurve=False
        # first process levels L0, L1, L2, L3
        for i in range(0,4):
            updateCurve=self._updateLevel(i, 2*i+1, updateCurve)
        # sustain point shares y-coordinate L3
        self.coordinates[2*4+1]=self.coordinates[2*3+1]
        # finally process L4, the release point
        updateCurve=self._updateLevel(4, 2*5+1, updateCurve)

        # first process times T1, T2, T3
        x=EnvelopeDisplay.ORIGINX
        for i in range(0,3):
            time=self.timeVar[self.operator][i].get()
            if time!="":
                x=x+int(time)+1
            else:
                # use old coordinates otherwise
                x=max(self.coordinates[2*i]+1, self.coordinates[2*i+2])
            if self.coordinates[2*i+2] != x:
                updateCurve=True
                self.coordinates[2*i+2]=x

        # Now process T4
        t4=self.timeVar[self.operator][3].get()
        if t4!="":
            x=EnvelopeDisplay.ORIGINX+400-1-int(t4)
            if self.coordinates[2*3+2] != x:
                updateCurve=True
                self.coordinates[2*3+2]=x
                
        if updateCurve:
            self.canvas.coords(self.ad1d2Curve, *self.coordinates[:8])
            self.canvas.coords(self.sLine, *self.coordinates[6:10])
            self.canvas.coords(self.rLine, *self.coordinates[8:])


# LED states in keyboard scaling display is -1 (all off) or led# 0..3 lit
ALL_LEDS_OFF=-1

class KeyboardDisplay(tkinter.Canvas):
    '''Represents the Keyboard-level Scaling Display'''
    
    _leftLedCoord  =  ((75, 109),  (61,  89),  (53,  48),  (70,  30))
    _rightLedCoord = ((179, 109), (194,  89), (195,  48), (180,  30))

    def __init__(self, parent, **kwargs):
        tkinter.Canvas.__init__(self, parent, kwargs)
        self.config(width=256,
                    height=160,
                    background=BACKGROUND_COLOR,
                    borderwidth='0',
                    highlightthickness='0')

        # background image, "graphics"
        img=getPhotoImage('kbd-scaling.png')
        self.create_image(0, 0, anchor=tkinter.NW, image=img)

        # LEDs
        img=getPhotoImage('led-on.png')
        leftLed=self.create_image(0, 0, anchor=tkinter.NW, image=img, state='hidden')
        rightLed=self.create_image(0, 0, anchor=tkinter.NW, image=img, state='hidden')
        self.leftHalf=HalfKeyboardDisplay(self, leftLed,
                                          KeyboardDisplay._leftLedCoord)
        self.rightHalf=HalfKeyboardDisplay(self, rightLed,
                                           KeyboardDisplay._rightLedCoord)

    def onParameterChanged(self):
        self.leftHalf.onParameterChanged()
        self.rightHalf.onParameterChanged()

    def setVariables(self, leftDepth, leftCurve, rightDepth, rightCurve):
        self.leftHalf.setVariables(leftDepth, leftCurve)
        self.rightHalf.setVariables(rightDepth, rightCurve)

class HalfKeyboardDisplay:
    '''Left/Right half of Keyboard-level Scaling Display'''
    def __init__(self, canvas, ledId, coordinates):
        self.canvas=canvas
        self.ledId=ledId
        self.coordinates=coordinates
        self.depthVar=None
        self.curveVar=None
        self.state=ALL_LEDS_OFF

    def setVariables(self, depthVar, curveVar):
        '''track new pair of variables (or None)'''
        self.depthVar=depthVar
        self.curveVar=curveVar

    def onParameterChanged(self):
        '''determine new state and update canvas if needed'''
        newState=self.state
        if self.depthVar is None or self.curveVar is None:
            newState=ALL_LEDS_OFF
        else:
            depth=self.depthVar.get()
            if depth!="":
                newState=ALL_LEDS_OFF if int(depth)==0 else int(self.curveVar.get())
        if newState != self.state:
            if newState==ALL_LEDS_OFF:
                self.canvas.itemconfig(self.ledId, state='hidden')
            else:
                if self.state==ALL_LEDS_OFF:
                    self.canvas.itemconfig(self.ledId, state='normal')
                (x,y)=self.coordinates[newState]
                self.canvas.moveto(self.ledId, x, y)
            self.state=newState
