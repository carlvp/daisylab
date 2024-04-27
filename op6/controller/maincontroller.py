class MainController:
    '''
    The main controller manages user interaction and mediates operations 
    between view and model.
    '''
    def __init__(self):
        # TODO: performanceController 
        # self.performanceController=PerformanceController(self)
        self.controllers={'MainController': self}
        # self.perfromanceController.registerControllerObjects(self.controllers)
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
        # TODO: performanceController
        # self.perfromanceController.setViews(views)

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
            # self.perfromanceController.clipboadChangedNotifier(clipboard)
