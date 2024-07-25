import alsa_midi
from alsa_midi import SequencerClient, READ_PORT, ProgramChangeEvent, \
                      ControlChangeEvent

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

    def _sendParameterNumber(self, channel, paramNumber, isRegistered):
        NRPN=98
        RPN=100
        fineCC=RPN if isRegistered else NRPN
        coarseCC=fineCC+1
        msb=paramNumber>>7
        lsb=paramNumber & 127
        # TODO: we can be smart about not sending redundant messages
        # cache basechannel, number, (NRPN/RPN)
        self.client.event_output(ControlChangeEvent(channel, coarseCC, msb))
        self.client.event_output(ControlChangeEvent(channel, fineCC, lsb))

    def sendParameterCoarse(self, channel, param, value, isRegistered=False):
        DataEntry=6
        self._sendParameterNumber(channel, param, isRegistered)
        self.client.event_output(ControlChangeEvent(channel, DataEntry, value))
        self.client.drain_output()
        
    def sendParameter(self, channel, param, value, isRegistered=False):
        '''
        sends a registered or non-registered parameter
        param and value are 14-bit unsigned integers
        '''
        DataEntryFine=38
        msb=value>>7
        lsb=value & 127
        # TODO: we can be smart about not sending redundant messages
        # cache basechannel, number and value (msb/lsb)
        self.sendParameterCoarse(channel, param, msb, isRegistered)
        self.sendMidiEvent(ControlChangeEvent(channel, DataEntryFine, lsb))

    def sendSysEx(self, payload):
        self.sendMidiEvent(alsa_midi.SysExEvent(payload))

    def chopUpSysEx(self, payload, chunkSize):
        i=1                # Skip first 0xf0, we will add it in each chunk
        end=len(payload)-1 # Skip last 0xf7, we will add it in each chunk
        buffer=bytearray(chunkSize+2)
        buffer[0]=0xf0
        buffer[chunkSize+1]=0xf7

        while end-i > chunkSize:
            buffer[1:chunkSize+1]=payload[i:i+chunkSize]
            self.sendSysEx(buffer)
            i=i+chunkSize
        lastChunk=end-i
        buffer[1:1+lastChunk]=payload[i:end]
        buffer[1+lastChunk]=0xf7
        self.sendSysEx(buffer[0:2+lastChunk])
