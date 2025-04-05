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

    def _createLayout(self):
        self._makeLabel("Channel", 0, 0, columnspan=2)
        self._makeLabel("Volume", 1, 0)
        self._makeScale("Volume", 2, 0)

        self._makeLabel("Pan", 1, 1)
        self._makeScale("Pan", 2, 1)
