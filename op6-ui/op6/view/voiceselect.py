import tkinter
from tkinter.filedialog import askopenfilename
from . import colorscheme

# interface (MainView):
# * registerModules()
# * resolveModules()
#
# interface (VoiceSelectController):
# * selectVoice()

BACKGROUND_COLOR=colorscheme.RETRO_DISPLAY_BACKGROUND
FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_FOREGROUND
LIGHT_FOREGROUND_COLOR=colorscheme.RETRO_DISPLAY_HIGHLIGHTED
    
class VoiceSelectScreen(tkinter.Frame):
    '''
    The UI of the voice-select mode:
    * MIDI Program change (i.e. voice) via a menu
    * Program banks (SysEx files) can be loaded
    '''
    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        
        # Voice Bank
        self.voiceBank=VoiceBankPanel(self)
        self.voiceBank.grid(row=1, column=0)
        self.voiceBank.config(background=colorscheme.RETRO_DISPLAY_BACKGROUND)
        self.selectedVoiceNumber=None
        self.controller=None
        
    def registerModules(self, modules):
        '''adds this view object to the module dictionary.'''
        modules['VoiceSelectScreen']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.controller=modules['VoiceSelectController']
        self.voiceBank.setController(self.controller)

    def selectVoice(self, voiceNumber):
        if voiceNumber!=self.selectedVoiceNumber:
            if not self.selectedVoiceNumber is None:
                self.voiceBank.setVoiceIsSelected(self.selectedVoiceNumber,
                                                  False)
            self.voiceBank.setVoiceIsSelected(voiceNumber, True)
            self.selectedVoiceNumber=voiceNumber

    def setVoiceName(self, voiceNumber, voiceName):
        self.voiceBank.setVoiceName(voiceNumber, voiceName)

    def setBankName(self, bankName):
        self.voiceBank.setBankName(bankName)

    
    def askSyxFilename(self):
        filename=askopenfilename(title="Load Voice Bank (syx)",
                                 filetypes=[("MIDI SysEx", "*.syx"),
                                            ("All files", "*")])
        return filename

class BankNamePanel(tkinter.Frame):

    def __init__(self, parent, **kwargs):
        tkinter.Frame.__init__(self, parent, kwargs)
        self.controller=None
        
        # bank name
        self.bankNameId=tkinter.Label(self,
                                      text="Bank: (no bank loaded)",
                                      anchor=tkinter.W,
                                      width=24,
                                      foreground=FOREGROUND_COLOR,
                                      background=BACKGROUND_COLOR)
        self.bankNameId.grid(row=0, column=0, sticky=tkinter.W)
        # load button
        button=tkinter.Button(self,
                              text="Load",
                              command=lambda: self.loadButtonHandler())
        button.grid(row=0, column=1, sticky=tkinter.E)

    def setController(self, controller):
        self.controller=controller
        
    def loadButtonHandler(self):
        self.controller.loadVoiceBank()

    def setBankName(self, name):
        self.bankNameId.config(text="Bank: "+name)

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

        # bank name
        self.bankNameId=BankNamePanel(self)
        self.bankNameId.grid(row=0, column=0, columnspan=2,
                           sticky=(tkinter.W, tkinter.E))
        self.bankNameId.config(background=BACKGROUND_COLOR)
        # create buttons
        self.voiceId=[]
        for k in range(2):
            for r in range(16):
                b=16*k + r
                id=tkinter.Button(self,
                                  text=voiceName_(b+1, 'INIT_VOICE'),
                                  width=13)
                id.grid(row=r+1, column=k)
                id.config(command=lambda i=b: self.buttonHandler(i)) 
                self.setButtonIsSelected_(id, False)
                self.voiceId.append(id);
                
    def setController(self, controller):
        self.controller=controller
        self.bankNameId.setController(controller)

    def buttonHandler(self, voiceNumber):
        self.controller.setVoice(voiceNumber)

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

    def setBankName(self, name):
        self.bankNameId.setBankName(name)
