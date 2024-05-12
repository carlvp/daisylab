import tkinter
from tkinter import ttk
from .performance import PerformanceScreen;
from .voice import VoiceEditorScreen;
from .resources import getPhotoImage
from . import colorscheme

class MainView:
    PERFORMANCE_SCREEN=0
    VOICE_EDITOR_SCREEN=1

    '''main view class, handles all things relating to Tk and user interface.'''
    def __init__(self):
        # root and tabbed screens
        self.root=tkinter.Tk()
        self.setTitle("Op6")
        self.root.config(background='black')
        
        # icon
        self.root.iconphoto(True, getPhotoImage('op6-64x64.png'))
        # tabbed screens
        self.screens=TabbedScreens(self.root)
        self.screens.grid(row=0, column=0, sticky=tkinter.N)
        self.screens.setReqDimensions(830, 509)
        self.screens.config(background='black')
        self.currScreen=None
        # Performance Screen
        self.performanceScreen=PerformanceScreen(self.screens)
        self.screens.add(self.performanceScreen)
        # Voice Editor Screen
        self.voiceEditorScreen=VoiceEditorScreen(self.screens)
        self.screens.add(self.voiceEditorScreen)
        # MenuButtons
        self.menuButtons=MenuButtons(self.root)
        self.menuButtons.grid(row=1, column=0, sticky=(tkinter.S,tkinter.W))
        # Register Views
        self.views={'MainView': self}
        self.performanceScreen.registerViewObjects(self.views)
        self.voiceEditorScreen.registerViewObjects(self.views)

    def run(self):
        '''runs Tk mainloop'''
        self.root.mainloop()

    def setTitle(self, title):
        '''sets the title of the topmost window'''
        self.root.title(title)

    def getViews(self):
        '''returns a dictionary containing the views'''
        return self.views
    
    def setControllers(self, controllers):
        '''
        sets the controllers of the various views
        controllers is a dictionary from controller name to instance
        '''
        self.menuButtons.setController(controllers['MainController'])
        self.performanceScreen.setControllers(controllers)
        self.voiceEditorScreen.setControllers(controllers)
        
    def selectScreen(self, index):
        if index!=self.currScreen:
            self.currScreen=index
            self.screens.select(index)
            self.menuButtons.updateSelectedScreen(index)

    def showErrorDialog(self, title, message, detail):
        tkinter.messagebox.showerror(title=title,
                                     message=message,
                                     detail=detail)

class TabbedScreens(tkinter.Frame):
    '''
    Homebrewn tabbed frames
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        self.tabs=[]
        self.currTab=None

    def add(self, frame):
        '''add frame to the collection of tabs'''
        frame.config(background='black')
        self.tabs.append(frame)

    def select(self, tabId):
        '''select tab (0,1,2,...) to display'''
        if self.currTab is not None:
            self.currTab.grid_remove()
        if tabId is None:
            self.currTab=None
        else:
            self.currTab=self.tabs[tabId]
            self.currTab.grid(row=0, column=0, sticky=tkinter.N)

    def setReqDimensions(self, width, height):
        '''Set the minimum/required width and height'''
        self.columnconfigure(0, minsize=width)
        self.rowconfigure(0, minsize=height)

    def getReqDimensions(self):
        '''Determine the maximum required dimensions among the tabs
           N.B: initially, the dimensions are not yet set'''
        w=1
        h=1
        for t in self.tabs:
            w=max(w, t.winfo_reqwidth())
            h=max(h, t.winfo_reqheight())
        return (w,h)

class MenuButtons(tkinter.Frame):
    BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
    FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
    LIGHT_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_HIGHLIGHTED
    
    '''Container for the MenuButtons'''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        
        self.controller=None
        buttonText=("Performance", "Voice Editor")
        self.buttons=[]
        for b in range(len(buttonText)):
            button=tkinter.Button(self, text=buttonText[b])
            button.grid(row=0, column=b)
            button.config(command=lambda id=b: self.buttonHandler(id))
            self.buttons.append(button)
        button=self.buttons[0]
        self.normalBg=button.cget('background')
        self.activeBg=button.cget('activebackground')

    def setController(self, controller):
        self.controller=controller

    def buttonHandler(self, id):
        if id==0 or id==1:
            self.controller.setActiveScreen(id)

    def updateSelectedScreen(self, id):
        self._updateEnabled(self.buttons[id], False)
        other=1-id
        self._updateEnabled(self.buttons[other], True)

    def _updateEnabled(self, button, isEnabled):
        fg=MenuButtons.FOREGROUND_COLOR
        bg=MenuButtons.BACKGROUND_COLOR
        light=MenuButtons.LIGHT_FOREGROUND_COLOR
        (state, fg, activeFg, bg)=(tkinter.NORMAL, fg, light, bg) if isEnabled \
                                 else (tkinter.DISABLED, bg, bg, fg)
        normalBg=button.config(state=state,
                               foreground=fg,
                               activeforeground=activeFg,
                               background=bg,
                               activebackground=bg)
