import alsa_midi
from alsa_midi import Address, \
    EventType, \
    ControlChangeEvent, \
    PortCaps, \
    PortType, \
    PortUnsubscribedEvent, \
    ProgramChangeEvent, \
    ResetEvent, \
    SequencerClient       
import threading

_ANNOUNCE_PORT=Address(0,1)
_MIDI_THRU_PORT_NAME="Midi Through Port-0"

class MidiAlsa:
    '''
    ALSA-based interface to the midi devices
    We imagine that the necessary support for other platforms (that don\'t
    use ALSA) could be implemented similarly. 
    '''

    def __init__(self, name):
        self.client=SequencerClient(name)
        self.port=self.client.create_port("in/out")
        self.listenerThread=None

    def startListen(self, listener):
        '''
        Start subscription to SYSTEM SND_SEQ_PORT_SYSTEM_ANNOUNCE port and
        start midi listener thread
        '''
        self.port.connect_from(_ANNOUNCE_PORT)
        listenerThread=threading.Thread(target=self.midiListener_,
                                        name="midi listener",
                                        args=(listener,),
                                        daemon=True)
        listenerThread.start()

    def shutDown(self):
        '''
        Close connection and release all resources. 
        This renders the output useless.
        '''
        self.client.close()
        self.port=None
        self.client=None

    def getPort(self):
        return Address(self.port)


    def findPort(self, startOfName):
        '''
        Find the port, whose name starts with the given prefix.
        If there are several matching ports, the first one is returned.
        If there is no matching port, None is returned
        '''
        for p in self.client.list_ports(output=True):
            if p.name.startswith(startOfName):
                return Address(p)
        return None

    def getMidiThruPort(self):
        return self.findPort(_MIDI_THRU_PORT_NAME)

    def listPorts(self,
                  input: bool=None,
                  output: bool=None,
                  include_system: bool=False,
                  include_midi_thru: bool=False):
        '''
        list of all ports that match criterion

        :param input: return ports usable for event input
        :param output: return ports usable for event output
        :param include_system: include system ports
        :param include_midi_through: include 'midi through' ports
        '''
        return [Address(p) for p in self.client.list_ports(input=input,
                                                           output=output,
                                                           include_system=include_system,
                                                           include_midi_through=include_midi_thru)]

    class PortInfoWrapper:
        def __init__(self, info):
            self.info=info

        def getPort(self):
            return Address(self.info.client_id, self.info.port_id)
        
        def getName(self):
            return self.info.name;

        def isInputPort(self):
            # yes: WRITE means the other endpoint writes (input) 
            return (self.info.capability & PortCaps.WRITE)==PortCaps.WRITE

        def isOutputPort(self):
            # yes: READ means the other endpoint reads (output) 
            return (self.info.capability & PortCaps.READ)==PortCaps.READ

        def isHardwarePort(self):
            return (self.info.type & PortType.HARDWARE)==PortType.HARDWARE

        def isGenericMidiPort(self):
            return (self.info.type & PortType.MIDI_GENERIC)==PortType.MIDI_GENERIC

    def getPortInfo(self, port):
        info=self.client.get_port_info(port)
        return MidiAlsa.PortInfoWrapper(info)
    
    def connectPorts(self, src, dest):
        self.client.subscribe_port(src, dest)

    def disconnectPorts(self, src, dest):
        self.client.unsubscribe_port(src, dest)

    def sendMidiEvent(self, event):
        self.client.event_output(event)
        self.client.drain_output()
        
    def sendProgramChange(self, channel, program):
        self.sendMidiEvent(ProgramChangeEvent(channel, program))

    def sendControlChange(self, channel, ccIndex, value):
        self.sendMidiEvent(ControlChangeEvent(channel, ccIndex, value))

    def sendReset(self):
        self.sendMidiEvent(ResetEvent())

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

    def midiListener_(self, listener):
        try:
            while True:
                event = self.client.event_input()
                if event.type==EventType.CONTROLLER:
                    listener.onControllerChange(event.channel, event.param, event.value)
                elif event.type==EventType.PGMCHANGE:
                    listener.onProgramChange(event.channel, event.value)
                elif event.type==EventType.PORT_START:
                    listener.onPortAdded(event.addr)
                elif event.type==EventType.PORT_EXIT:
                    listener.onPortRemoved(event.addr)
        except:
            pass # exit midi listener thread
