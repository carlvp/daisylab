from alsa_midi import SequencerClient, READ_PORT, ProgramChangeEvent

class AlsaMidiOutput:
    '''
    ALSA-based interface to the midi devices
    We imagine that the necessary support for other platforms (that don\'t
    use ALSA) could be implemented similarly. 
    '''

    def __init__(self, name):
        self.client=SequencerClient(name)
        # yes, the output port has caps=READ_PORT
        # I suppose this is looking at the capabilities from the other end
        # of the subscription (we write, another client reads)...
        self.port=self.client.create_port("output", caps=READ_PORT)

    def shutDown(self):
        '''
        Close connection and release all resources. 
        This renders the output useless.
        '''
        self.client.close()

    def findPort(self, startOfName):
        '''
        Find the port, whose name starts with the given prefix.
        If there are several matching ports, the first one is returned.
        If there is no matching port, None is returned
        '''
        for pinfo in self.client.list_ports(output=True):
            if pinfo.name.startswith(startOfName):
                return pinfo
        return None

    def connectTo(self, port):
        self.port.connect_to(port)

    def sendMidiEvent(self, event):
        self.client.event_output(event)
        self.client.drain_output()
        
    def sendProgramChange(self, channel, program):
        self.sendMidiEvent(ProgramChangeEvent(channel, program))
