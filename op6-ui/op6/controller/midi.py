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
        self.midiThruPort=None
        self.connectedInputs=[]

    def getMidiOut(self):
        return self.midi

    def startUp(self):
        # Identify and connect MIDI ports
        self.startMidiPorts_()
        # register this instance as a MIDI Listener
        self.midi.startListen(self)

    def setOperationalMode(self, mode):
        SWITCH_MODE=7*128
        self.midi.sendParameter(self.baseChannel, SWITCH_MODE, 128*mode)

    def shutDown(self):
        self.shutDownMidiPorts_()
        self.midi.shutDown()

    #
    # MIDI Listener implementation
    #
    def onPortAdded(self, port):
        # print("Port added:  ", repr(port))
        if self.op6Port is None and self.isOp6MidiPort_(port):
            self.connectOp6_(port)
        elif self.isHardwareMidiInput_(port):
            self.connectMidiInput_(port)
    
    def onPortRemoved(self, port):
        # print("Port removed: ", repr(port))
        if port==self.op6Port:
            self.op6Port=None
        elif port in self.connectedInputs:
            self.connectedInputs.remove(port)

    #
    # Helpers
    #
    def startMidiPorts_(self):
        # Connect MIDI Thru to Op6 (Daisy Seed)
        self.midiThruPort=self.midi.getMidiThruPort()
        op6Port=self.midi.findPort(OP6_PORT_PREFIX_)
        if op6Port is not None and self.isOp6MidiPort_(op6Port):
            self.connectOp6_(op6Port)
        # Connect all H/W input ports (except Op6/Daisy Seed) to MIDI Thru
        for p in self.midi.listPorts(input=True,
                                     output=False,
                                     include_system=False,
                                     include_midi_thru=False):
            if p != op6Port and self.isHardwareMidiInput_(p):
                self.connectMidiInput_(p)

    def shutDownMidiPorts_(self):
        # Disconnect the midi ports: Op6 UI and MIDI Thru to Op6/Daisy Seed
        if self.op6Port is not None:
            uiPort=self.midi.getPort()
            self.midi.disconnectPorts(uiPort, self.op6Port)
            if self.midiThruPort is not None:
                self.midi.disconnectPorts(self.midiThruPort, self.op6Port)
        # Disconnect input ports -> MIDI Thru
        if self.midiThruPort is not None:
            for p in self.connectedInputs:
                self.midi.disconnectPorts(p, self.midiThruPort)

    def isOp6MidiPort_(self, port):
        portInfo=self.midi.getPortInfo(port)
        return (portInfo.isHardwarePort() and portInfo.isInputPort() and
                portInfo.isGenericMidiPort() and
                portInfo.getName().startswith(OP6_PORT_PREFIX_))

    def isHardwareMidiInput_(self, port):
        portInfo=self.midi.getPortInfo(port)
        return (portInfo.isHardwarePort() and portInfo.isOutputPort() and
                portInfo.isGenericMidiPort())

    def connectOp6_(self, op6InputPort):
        # Connect op6 UI to Op6 (Daisy Seed)
        self.op6Port=op6InputPort
        self.midi.connectPorts(self.midi.getPort(), op6InputPort)
        # Connect midiThru port (other inputs) to Op6 (Daiosy Seed)
        if self.midiThruPort is not None:
            self.midi.connectPorts(self.midiThruPort, op6InputPort)

    def connectMidiInput_(self, inputPort):
        # Connect input to midiThru, which is directed to Op6 (Daisy Seed) and Op6 UI
        if self.midiThruPort is not None and inputPort not in self.connectedInputs:
            #print("Connect "+str(inputPort))
            self.midi.connectPorts(inputPort, self.midiThruPort)
            self.connectedInputs.append(inputPort)

