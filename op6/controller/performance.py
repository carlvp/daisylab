# interface (MainController):
# registerControllerObjects()
# setView()
# initUI()
#
# interface (View)
# setVoice(voiceNumber)

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
        self.performanceScreen.setVoiceName(10, "E.PIANO 1")
        
    def setVoice(self, voiceNumber):
        '''called from PerformanceScreen to set new voice'''
        self.performanceScreen.selectVoice(voiceNumber)
        self.currVoice=voiceNumber
