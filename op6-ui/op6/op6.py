from .view.mainview import MainView
from .controller.maincontroller import MainController

class Op6App:
    '''Op6App: comprising its model, view and controller'''

    def __init__(self):
        self.view=MainView()
        self.controller=MainController()

    def startUp_(self):
        # Register Modules
        modules={}
        self.view.registerModules(modules)
        self.controller.registerModules(modules)
        # Connect modules
        self.controller.resolveModules(modules)
        self.view.resolveModules(modules)
        # Initialize user interface
        self.controller.initUI()
        # Start displaying the Performance Screen
        self.controller.setActiveScreen(MainView.VOICE_SELECT_SCREEN)
        self.controller.startUp()

    def run(self):
        '''runs op6 and blocks until app exits'''
        self.startUp_()
        self.view.run()
        self.shutDown_()
        
    def shutDown_(self):
        '''releases all resources held'''
        self.controller.shutDown()

def main(argv):
    app=Op6App()
    app.run()
    return 0

