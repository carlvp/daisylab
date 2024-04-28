import os
from op6.model.syx import SyxPacked32Voice

# interface (MainController):
# registerControllerObjects()
# setView()
# initUI()
#
# interface (View)
# setVoice(voiceNumber)
# loadVoiceBank()

class PerformanceController:
    '''
    The PerformanceController manages the UI of the PerformanceScreen
    and mediates user interaction and operations in the performace mode
    '''

    def __init__(self):
        self.performanceScreen=None
        self.currVoice=None
        
    def registerControllerObjects(self, controllers):
        '''adds controller objects to the dictionary, controllers.'''
        controllers['PerformanceController']=self

    def setViews(self, views):
        '''connects to relevant views in the dictionary, views'''
        self.performanceScreen=views['PerformanceScreen']

    def initUI(self):
        self.currVoice=0
        self.performanceScreen.selectVoice(0)

    def setVoice(self, voiceNumber):
        '''called from PerformanceScreen to set new voice'''
        self.performanceScreen.selectVoice(voiceNumber)
        self.currVoice=voiceNumber

    def loadVoiceBank(self):
        '''called from the PerformanceScreen to load a voice bank'''
        filename=self.performanceScreen.askSyxFilename()
        if filename==() or filename=="":
            return # load cancelled
        
        syx=SyxPacked32Voice.load(filename)
        if syx is None:
            return # load failed (not a well-formed SyxPacked32Voice file)

        (bankName,_)=os.path.splitext(os.path.basename(filename))
        self.performanceScreen.setBankName(bankName[:24])
        for n in range(32):
            self.performanceScreen.setVoiceName(n, syx.getVoice(n).getName()) 
