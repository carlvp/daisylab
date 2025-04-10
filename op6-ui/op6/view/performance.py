import tkinter
from . import colorscheme

# interface (MainView):
# * registerModules()
# * resolveModules()
#
# interface (PerformanceController):
# * setPerformanceParameter()

BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
LIGHT_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_HIGHLIGHTED
DARK_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_DARK

class PerformanceScreen(tkinter.Frame):
    '''
    The UI of the voice-edit mode
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)

        self.controller=None
        self.parameterValue={}
        self._createLayout()
        
    def registerModules(self, modules):
        '''adds this view object to the module dictionary.'''
        modules['PerformanceScreen']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.controller=modules['PerformanceController']

    def setPerformanceParameter(self, paramName, paramValue):
        '''sets a parameter value: used by controller, reflected in UI'''
        var=self.parameterValue[paramName]
        var.set(paramValue)
        self._extraUpdateAction(paramName, paramValue)

    def _onPerformanceParamChanged(self, paramName, *_):
        '''listens to parameter values changed via UI'''
        var=self.parameterValue[paramName]
        paramValue=var.get()
        self.controller.updatePerformanceParameter(paramName, paramValue)
        self._extraUpdateAction(paramName, paramValue)

    def _extraUpdateAction(self, paramName, paramValue):
        '''some parameters have extra update actions, handled here'''
        pass

    def _makeLabel(self, text, row, column, columnspan=1):
        id=tkinter.Label(self,
                         text=text,
                         foreground=FOREGROUND_COLOR,
                         background=BACKGROUND_COLOR,
                         anchor=tkinter.N)
        id.grid(row=row, column=column, columnspan=columnspan)
        return id

    def _makeScale(self, paramName, row, column,
                   minValue=0, maxValue=127, rangeInPixels=128):
        SLIDER_LENGTH=16

        var=tkinter.StringVar(master=self, value='0', name=paramName)
        var.trace_add("write", self._onPerformanceParamChanged)
        id=tkinter.Scale(self,
                         variable=var,
                         from_=maxValue, to=minValue,
                         sliderlength=SLIDER_LENGTH, length=rangeInPixels+SLIDER_LENGTH)
        id.grid(row=row, column=column)
        id.config(foreground=FOREGROUND_COLOR,
                  background=BACKGROUND_COLOR,
                  borderwidth='1',
                  highlightthickness='0',
                  troughcolor=BACKGROUND_COLOR)
        self.parameterValue[paramName]=var
        return id

    def _makeCombobox(self, paramName, values, width, row, column, columnspan=1, initValue=0):
        var=tkinter.StringVar(master=self, value=values[initValue], name=paramName)
        var.trace_add("write", self._onPerformanceParamChanged)
        id=RetroCombobox(self, var, values, width=width)
        id.grid(row=row, column=column, columnspan=columnspan, sticky=tkinter.N)
        cboxFormatter=ComboboxFormatter(var, values)
        self.parameterValue[paramName]=cboxFormatter
        return id

    def _createLayout(self):
        self._makeLabel("Channel", 0, 0, columnspan=2)
        self._makeLabel("Volume", 1, 0)
        self._makeScale("Volume", 2, 0)

        self._makeLabel("Pan", 1, 1)
        self._makeScale("Pan", 2, 1)

        self._makeLabel("Poly/", 0, 2)
        self._makeLabel("Mono",  1, 2)
        self._makeCombobox("Poly", ("Mono", "Poly"), 4, 2, 2)

        self._makeLabel("Porta", 0, 3)
        self._makeLabel("Time",  1, 3)
        self._makeScale("PortaTime", 2, 3)

        self._makeLabel("Porta", 3, 3)
        self._makeLabel("Mode", 4, 3)
        self._makeCombobox("PortaMode", ("Off", "Legato", "On"), 6, 5, 3)

        self._makeLabel("PBend", 0, 4)
        self._makeLabel("Range",  1, 4)
        self._makeScale("PBendRange", 2, 4)

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

    def buttonHandler(self):
        if self.focus_get()!=self:
            self.focus_set()
        n=self.values.index(self.var.get())+1
        if n==len(self.values):
            n=0
        self.var.set(self.values[n])

class ComboboxFormatter:
    '''Formats a 0..N-1 as the values[i] strings'''
    def __init__(self, var, values):
        self.var=var
        self.values=values

    def set(self, value):
        self.var.set(self.values[int(value)])

    def get(self):
        return self.values.index(self.var.get())
