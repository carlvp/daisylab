class DisplayController:
    '''
    The Display Controller manages the 2x16 display
    '''

    def __init__(self):
        self.line=["                ",
                   "                "]
        self.onScreenDisplay=None

    def registerModules(self, modules):
        '''adds this view object to the module dictionary.'''
        modules['DisplayController']=self

    def resolveModules(self, modules):
        '''connects to relevant modules in the module dictionary'''
        self.onScreenDisplay=modules['OnScreenDisplay']

    def update(self, line1=None, line2=None):
        if line1 is not None:
            line1=line1.ljust(16, ' ')
            self.line[0]=line1
        if line2 is not None:
            line2=line2.ljust(16, ' ')
            self.line[1]=line2
        print(f"|{self.line[0]}|")
        print(f"|{self.line[1]}|")
        print("+----------------+")
        self.onScreenDisplay.update(line1, line2)
