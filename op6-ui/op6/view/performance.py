import tkinter
from . import colorscheme
from .resources import getPhotoImage

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

    def _makeImage(self, name, row, column, rowspan=1, columnspan=1):
        img=getPhotoImage(name)
        id=tkinter.Label(self,
                         image=img,
                         anchor=tkinter.N)
        id.grid(row=row, column=column, rowspan=rowspan, columnspan=columnspan)
        id.config(foreground=FOREGROUND_COLOR,
                  background=BACKGROUND_COLOR,
                  borderwidth='0',
                  highlightthickness='0')
        return id

    def _createLayout(self):
        self._makeImage("channel-controls.png", 0, 0, columnspan=5)
        self._makeImage("box-volume.png", 1, 0)
        self._makeScale("Volume", 2, 0)
        self._makeImage("box-poly-mono.png", 4, 0)
        self._makeCombobox("Poly", ("Mono", "Poly"), 4, 5, 0)

        self._makeImage("box-pan.png", 1, 1)
        self._makeScale("Pan", 2, 1)

        self._makeImage("box-porta-time.png", 1, 2);
        self._makeScale("PortaTime", 2, 2)

        self._makeImage("box-porta-mode.png", 4, 2)
        self._makeCombobox("PortaMode", ("Off", "Legato", "On"), 6, 5, 2)

        self._makeImage("box-pbend-range.png", 1, 3)
        self._makeScale("PBendRange", 2, 3)

        self._makeImage("box-blank.png", 1, 4)
        
        self._makeImage("dest-lfo-pitch.png", 5, 5)
        self._makeImage("dest-pitch-bend.png", 6, 5)
        self._makeImage("dest-lfo-amp.png", 7, 5)
        self._makeImage("dest-amp-bias.png", 8, 5)

        self._makeImage("modulation-depth.png", 0, 6, columnspan=4)
        self._makeImage("box-mod-wheel.png", 1, 6)
        self._makeScale("ModRange", 2, 6)
        self._makeImage("modulation-routing.png", 3, 6, columnspan=4)
        self._makeImage("box-mod-wheel.png", 4, 6)
        self._makeCombobox("Mod2LfoPm", ("Off", "On"), 3, 5, 6)
        self._makeCombobox("Mod2LfoAm", ("Off", "On"), 3, 7, 6)
        self._makeCombobox("Mod2AmpBias", ("Off", "On"), 3, 8, 6)

        self._makeImage("box-atouch.png", 1, 7)
        self._makeScale("PressRange", 2, 7)
        self._makeImage("box-atouch.png", 4, 7)
        self._makeCombobox("Press2LfoPm", ("Off", "On"), 3, 5, 7)
        self._makeCombobox("Press2PBend", ("Off", "On"), 3, 6, 7)
        self._makeCombobox("Press2LfoAm", ("Off", "On"), 3, 7, 7)
        self._makeCombobox("Press2AmpBias", ("Off", "On"), 3, 8, 7)

        self._makeImage("box-blank.png", 1, 8)

        self._makeImage("box-blank.png", 1, 9)

        self._makeImage("delay-fx.png", 9, 0, columnspan=4)

        self._makeImage("box-delay-level.png", 10, 0)
        self._makeScale("DelayLevel", 11, 0)

        self._makeImage("box-feedback.png", 10, 1)
        self._makeScale("DelayFeedback", 11, 1)

        self._makeImage("box-delay-time.png", 10, 2)
        self._makeScale("DelayTime", 11, 2)

        self._makeImage("box-damping.png", 10, 3)
        self._makeScale("DelayDamp", 11, 3)

        
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
