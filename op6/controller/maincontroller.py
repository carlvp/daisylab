from .performance import PerformanceController
from .voice import VoiceEditorController

class MainController:
    '''
    The main controller manages user interaction and mediates operations 
    between view and model.
    '''
    def __init__(self):
        self.performanceController=PerformanceController()
        self.voiceEditorController=VoiceEditorController()
        
        self.controllers={'MainController': self}
        self.performanceController.registerControllerObjects(self.controllers)
        self.voiceEditorController.registerControllerObjects(self.controllers)
        self.clipboard=None

    def getControllers(self):
        '''returns a dictionary containing the controllers'''
        return self.controllers

    def setViews(self, views):
        '''
        sets the views of the various controllers
        views is a dictionary from controller name to instance
        '''
        self.view=views['MainView']
        self.performanceController.setViews(views)
        self.voiceEditorController.setViews(views)

    def initUI(self):
        self.performanceController.initUI()
        
    def setActiveScreen(self, index):
        # TODO: performanceController
        # self.performanceController.setHasActiveScreen(index==0)
        # self.otherController.setHasActiveScreen(index==1)
        self.view.selectScreen(index)

    def setClipboard(self, clipboard):
        '''
        set clipboard (or clear it using clipboard=None)
        '''
        if self.clipboard!=clipboard:
            self.clipboard=clipboard
            # TODO: notify all controllers
            # self.performanceController.clipboadChangedNotifier(clipboard)
