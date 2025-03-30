from op6.model.midialsa import MidiAlsa

# Connect to a port, whose name starts with this prefix
OP6_PORT_PREFIX_="Daisy Seed"

# Operational modes
PERFORMANCE_MODE=0
EDIT_MODE=1
COMPARE_MODE=2

class MidiController:
    '''
    The MIDI Controller manages MIDI input (and output to some degree)
    '''
    def __init__(self):
        self.midi=MidiAlsa("Op6 App")
        self.program=None
        self.baseChannel=0
        self.op6Port=None

    def getMidiOut(self):
        return self.midi

    def startUp(self):
        # Connect to OP6 if present
        daisysPort=self.midi.findPort(OP6_PORT_PREFIX_)
        if daisysPort is not None and self.isOp6MidiPort_(daisysPort):
            self.connectOp6MidiPort_(daisysPort)
        # register this instance as a MIDI Listener
        self.midi.startListen(self)

    def setOperationalMode(self, mode):
        SWITCH_MODE=7*128
        self.midi.sendParameter(self.baseChannel, SWITCH_MODE, 128*mode)

    def shutDown(self):
        self.midi.shutDown()

    #
    # MIDI Listener implementation
    #
    def onPortAdded(self, port):
        # print("Port added:  ", repr(port))
        if self.op6Port is None and self.isOp6MidiPort_(port):
            self.connectOp6MidiPort_(port)

    def onPortRemoved(self, port):
        # print("Port removed: ", repr(port))
        if port==self.op6Port:
            self.op6Port=None

    #
    # Helpers
    #
    def isOp6MidiPort_(self, port):
        portInfo=self.midi.getPortInfo(port)
        return (portInfo.isHardwarePort() and portInfo.isOutputPort() and
                portInfo.isGenericMidiPort() and
                portInfo.getName().startswith(OP6_PORT_PREFIX_))

    def connectOp6MidiPort_(self, port):
        self.op6Port=port
        self.midi.connectTo(port)

