import tkinter
from . import colorscheme

# interface (MainView):
# * registerViewObjects()
# * setControllers()
#
# interface (PerformanceController):
# * selectVoice()

class PerformanceScreen(tkinter.Frame):
    '''
    The UI of the performance mode:
    * Channel settings (CC), such as volume and pan can be modified
    * MIDI Program change (i.e. voice) via a menu
    * Program banks (SysEx files) can be loaded
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        
        # Voice Bank
        self.voiceBank=VoiceBankPanel(self)
        self.voiceBank.grid(row=0, column=0)
        self.voiceBank.config(background=colorscheme.RETRO_DISPLAY_BACKGROUND)
        self.selectedVoiceNumber=None
        
    def registerViewObjects(self, views):
        '''adds view objects to the dictionary, views.'''
        views['PerformanceScreen']=self

    def setControllers(self, controllers):
        performanceController=controllers['PerformanceController']
        self.controller=performanceController
        self.voiceBank.setController(performanceController)

    def selectVoice(self, voiceNumber):
        if voiceNumber!=self.selectedVoiceNumber:
            if not self.selectedVoiceNumber is None:
                self.voiceBank.setVoiceIsSelected(self.selectedVoiceNumber,
                                                  False)
            self.voiceBank.setVoiceIsSelected(voiceNumber, True)
            self.selectedVoiceNumber=voiceNumber

    def setVoiceName(self, voiceNumber, voiceName):
        self.voiceBank.setVoiceName(voiceNumber, voiceName)
        
def voiceName_(pgm, name):
        return f'{pgm:2d} {name:10}'

class VoiceBankPanel(tkinter.Frame):
    '''Panel, from which the voices can be selected'''

    BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
    FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
    LIGHT_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_HIGHLIGHTED
    
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        self.controller=None
        self.currVoiceId=None
        # create buttons
        self.voiceId=[]
        for k in range(2):
            for r in range(16):
                b=16*k + r
                id=tkinter.Button(self,
                                  text=voiceName_(b+1, 'INIT_VOICE'),
                                  width=13)
                id.grid(row=r, column=k)
                id.config(command=lambda i=b: self.buttonHandler(i)) 
                self.setButtonIsSelected_(id, False)
                self.voiceId.append(id);
                
    def setController(self, controller):
        self.performanceController=controller

    def buttonHandler(self, voiceNumber):
        self.performanceController.setVoice(voiceNumber)

    def setVoiceIsSelected(self, voiceNumber, isSelected):
        button=self.voiceId[voiceNumber]
        self.setButtonIsSelected_(button, isSelected)
        
    def setButtonIsSelected_(self, button, isSelected):
        # selected button is disabled and inverted
        fg=VoiceBankPanel.FOREGROUND_COLOR
        bg=VoiceBankPanel.BACKGROUND_COLOR
        light=VoiceBankPanel.LIGHT_FOREGROUND_COLOR
        if isSelected:
            button.config(state=tkinter.DISABLED,
                          background=fg,
                          foreground=bg,
                          activebackground=fg,
                          activeforeground=bg)
        else:
            button.config(state=tkinter.NORMAL,
                          background=bg,
                          foreground=fg,
                          activebackground=bg,
                          activeforeground=light)

    def setVoiceName(self, voiceNumber, voiceName):
        button=self.voiceId[voiceNumber]
        button.config(text=voiceName_(voiceNumber+1, voiceName))
