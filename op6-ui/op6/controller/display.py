class DisplayController:
    '''
    The Display Controller manages the 2x16 display
    '''

    def __init__(self):
        self.line=["                ",
                   "                "]

    def update(line1=None, line2=None):
        if line1 is not None:
            self.line[0]=line1
        if line2 is not None:
            self.line[1]=line2
        print(f"|{self.line[0]:16.16}|")
        print(f"|{self.line[1]:16.16}|")
        print("+----------------+")
