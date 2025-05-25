import tkinter
from . import colorscheme
from .resources import getPhotoImage

# interface (MainView):
# * registerModules()
# * resolveModules()
#
# interface (DisplayController):
# * TBD

BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
RETRO_DISPLAY_VERY_DARK=colorscheme.RETRO_DISPLAY_VERY_DARK
LCD_DARK_GREEN='#09180c'
LCD_DARK_GRAY='#080808'
LCD_GRAY1='#0c0c0c'
LCD_GRAY2='#101010'
LCD_GRAY3='#1c1c1c'
LCD_GRAY4='#2a2a2a'
LCD_GRAY5='#3c3c3c'

NUM_ROWS=2
NUM_COLUMNS=16

class OnScreenDisplay(tkinter.Frame):
    '''
    An on-screen widget, which doubles for the front-panel LCD display
    '''

    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        self.config(background=RETRO_DISPLAY_VERY_DARK)

        self._makeFrame()

        self.cells=[]
        for r in range(NUM_ROWS):
            line=[]
            for c in range(NUM_COLUMNS):
                id=tkinter.Label(self,
                                 text='X',
                                 foreground=FOREGROUND_COLOR,
                                 background=RETRO_DISPLAY_VERY_DARK,
                                 borderwidth=0)
                id.grid(row=r+1, column=c+1)
                line.append(id)
            self.cells.append(line)
        # equal weight
        self.rowconfigure(1, minsize=24)
        self.rowconfigure(2, minsize=24)
        for c in range(NUM_COLUMNS):
            self.columnconfigure(c+1, minsize=16)

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

    def _makeImage(self, name, row, column, rowspan=1, columnspan=1):
        img=getPhotoImage(name)
        id=tkinter.Label(self, image=img)
        id.grid(row=row, column=column, rowspan=rowspan, columnspan=columnspan)
        id.config(foreground=FOREGROUND_COLOR,
                  background=BACKGROUND_COLOR,
                  borderwidth='0',
                  highlightthickness='0')
        return id

    def _makeCanvas(self, canvas, row, column, rowspan=1, columnspan=1):
        sticky=(tkinter.W, tkinter.E) if canvas.isHorizontal else (tkinter.N, tkinter.S)
        canvas.grid(row=row, column=column,
                    rowspan=rowspan,
                    columnspan=columnspan,
                    sticky=sticky)
        return canvas

    def _makeFrame(self):
        self._makeImage("lcd-nw.png", 0, 0)
        self._makeCanvas(NorthCanvas(self, width=30, height=30), 0, 1, columnspan=16)
        self._makeImage("lcd-ne.png", 0, 17)
        self._makeCanvas(WestCanvas(self, width=30, height=30), 1, 0, rowspan=2)
        self._makeCanvas(EastCanvas(self, width=30, height=30), 1, 17, rowspan=2)
        self._makeImage("lcd-sw.png", 3, 0)
        self._makeCanvas(SouthCanvas(self, width=30, height=30), 3, 1, columnspan=16)
        self._makeImage("lcd-se.png", 3, 17)

class LcdCanvas(tkinter.Canvas):
    def __init__(self, parent, **kwargs):
        tkinter.Canvas.__init__(self, parent, **kwargs)
        self.config(background=BACKGROUND_COLOR,
                    borderwidth='0',
                    highlightthickness='0')

class NorthCanvas(LcdCanvas):
    def __init__(self, parent, **kwargs):
        LcdCanvas.__init__(self, parent, **kwargs)
        self.isHorizontal=True
        W=999 # sufficient so that we don't need to bother about resize
        self.create_line(0, 1, W, 1, fill=LCD_GRAY2, width=3)
        self.create_line(0, 4, W, 4, fill=LCD_GRAY3)
        self.create_line(0, 5, W, 5, fill=LCD_GRAY4)
        self.create_line(0, 12, W, 12, fill=LCD_GRAY3)
        self.create_line(0, 15, W, 15, fill=LCD_GRAY3, width=3)
        self.create_line(0, 18.5, W, 18.5, fill=LCD_DARK_GRAY, width=4)
        self.create_line(0, 26, W, 26, fill=RETRO_DISPLAY_VERY_DARK, width=7)
        self.create_line(0, 23, W, 23, fill=LCD_DARK_GREEN)

class WestCanvas(LcdCanvas):
    def __init__(self, parent, **kwargs):
        LcdCanvas.__init__(self, parent, **kwargs)
        self.isHorizontal=False
        H=999 # sufficient so that we don't need to bother about resize
        self.create_line(1, 0, 1, H, fill=LCD_GRAY2, width=3)
        self.create_line(3, 0, 3, H, fill=LCD_DARK_GRAY)
        self.create_line(9, 0, 9, H, fill=LCD_DARK_GRAY)
        self.create_line(10.5, 0, 10.5, H, fill=LCD_GRAY3, width=2)
        self.create_line(14, 0, 14, H, fill=LCD_GRAY2)
        self.create_line(22, 0, 22, H, fill=RETRO_DISPLAY_VERY_DARK, width=15)

class EastCanvas(LcdCanvas):
    def __init__(self, parent, **kwargs):
        LcdCanvas.__init__(self, parent, **kwargs)
        self.isHorizontal=False
        H=999 # sufficient so that we don't need to bother about resize
        self.create_line(8, 0, 8, H, fill=RETRO_DISPLAY_VERY_DARK, width=17)
        self.create_line(22, 0, 22, H, fill=LCD_DARK_GRAY, width=3)
        self.create_line(28, 0, 28, H, fill=LCD_GRAY1, width=3)

class SouthCanvas(LcdCanvas):
    def __init__(self, parent, **kwargs):
        LcdCanvas.__init__(self, parent, **kwargs)
        self.isHorizontal=True
        W=999 # sufficient so that we don't need to bother about resize
        self.create_line(0, 3, W, 3, fill=RETRO_DISPLAY_VERY_DARK, width=7)
        self.create_line(0, 7, W, 7, fill=LCD_GRAY3)
        self.create_line(0, 10, W, 10, fill=LCD_GRAY2, width=5)
        self.create_line(0, 13, W, 13, fill=LCD_DARK_GRAY)
        self.create_line(0, 18, W, 18, fill=LCD_GRAY2)
        self.create_line(0, 19, W, 19, fill=LCD_GRAY3)
        self.create_line(0, 21, W, 21, fill=LCD_GRAY3)
        self.create_line(0, 22, W, 22, fill=LCD_GRAY5)
        self.create_line(0, 23, W, 23, fill=LCD_GRAY3)
        self.create_line(0, 24, W, 24, fill=LCD_DARK_GRAY)
        self.create_line(0, 28, W, 28, fill=LCD_DARK_GRAY)
