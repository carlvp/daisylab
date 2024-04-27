from .view.mainview import MainView
from .controller.maincontroller import MainController

class Op6App:
    '''Op6App: comprising its model, view and controller'''

    def __init__(self):
        self.view=MainView()
        self.controller=MainController()
        # Create links View<-->Controllers
        self.view.setControllers(self.controller.getControllers())
        self.controller.setViews(self.view.getViews())
        # Initialize user interface
        self.controller.initUI()
        # Start displaying the Performance Screen
        self.controller.setActiveScreen(MainView.PERFORMANCE_SCREEN)

    def run(self):
        '''runs HTraxxorApp and blocks until app exits'''
        self.view.run()


def main(argv):
    app=Op6App()
    app.run()
    return 0

