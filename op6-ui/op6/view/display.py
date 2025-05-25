import tkinter
from . import colorscheme

# interface (MainView):
# * registerModules()
# * resolveModules()
#
# interface (DisplayController):
# * TBD

BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
NUM_ROWS=2
NUM_COLUMNS=16

class OnScreenDisplay(tkinter.Frame):
    '''
    An on-screen widget, which doubles for the front-panel LCD display
    '''

    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        self.config(background=BACKGROUND_COLOR)
        
        self.cells=[]
        for r in range(NUM_ROWS):
            line=[]
            for c in range(NUM_COLUMNS):
                id=tkinter.Label(self,
                                 text='X',
                                 foreground=FOREGROUND_COLOR,
                                 background=BACKGROUND_COLOR,
                                 borderwidth=0,
                                 anchor=tkinter.N)
                id.grid(row=r, column=c)
                line.append(id)
            self.cells.append(line)
        # equal weight
        self.rowconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)
        for c in range(NUM_COLUMNS):
            self.columnconfigure(c, minsize=16)

    def registerModules(self, modules):
        '''adds this view object to the module dictionary.'''
        modules['OnScreenDisplay']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        pass

    def update(self, line1=None, line2=None):
        if line1 is not None:
            self._updateLine(self.cells[0], line1)
        if line2 is not None:
            self._updateLine(self.cells[1], line2)

    def _updateLine(self, cells, s):
        n=min(len(s), NUM_COLUMNS)
        for c in range(n):
            cells[c].config(text=s[c])
