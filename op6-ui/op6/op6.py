from .view.mainview import MainView
from .controller.maincontroller import MainController
from .model.midialsa import AlsaMidiOutput

class Op6App:
    '''Op6App: comprising its model, view and controller'''

    def __init__(self):
        self.midiOut=AlsaMidiOutput("Op6 App")
        self.view=MainView()
        self.controller=MainController()

    def startUp_(self):
        # Create links View<-->Controllers
        self.view.setControllers(self.controller.getControllers())
        self.controller.setViews(self.view.getViews())
        # Initialize user interface
        self.controller.initUI()
        # Start displaying the Performance Screen
        self.controller.setActiveScreen(MainView.PERFORMANCE_SCREEN)
        # Connect to Daisy Seed (op6)
        daisysPort=self.midiOut.findPort("Daisy Seed")
        if not daisysPort is None:
            print("Connected to "+daisysPort.name)
            self.midiOut.connectTo(daisysPort)
            self.controller.setMidiOut(self.midiOut)

    def run(self):
        '''runs op6 and blocks until app exits'''
        self.startUp_()
        self.view.run()
        self.shutDown_()
        
    def shutDown_(self):
        '''releases all resources held'''
        self.midiOut.shutDown()

def main(argv):
    app=Op6App()
    app.run()
    return 0

