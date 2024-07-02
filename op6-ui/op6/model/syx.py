import os

class SyxPacked32Voice:
    def __init__(self, data):
        self.rawSyxData=data

    def getRawData(self):
        return self.rawSyxData

    def getVoice(self, n):
        i=6+128*n
        return (SyxPacked32Voice.VoiceData(self.rawSyxData[i:i+128])
                if n>=0 and n<=32
                else None)

    def corruptSyx_(data):
        '''sanity checks the integrity of the data'''
        # check basic properties of the format, including the header
        if (data is None or len(data)!=4104 or
            data[0:6]!=bytes([0xf0, 0x43, 0x00, 0x09, 0x20, 0x00])
            or data[4103]!=0xf7):
            return "Wrong data format"
        # check that all data is in range
        for b in data[6:4103]:
            if b>127:
                return "MIDI byte out-of-range"
        # check that ASCII data (voice names) are printable
        for n in range(32):
            i=6+128*n
            if not data[i+118:i+128].decode().isprintable():
                return f'Voice name {n+1} not printable'

        return None
    
    
    def load(filename, dialogManager):
        '''loads SyxPacked32Voice data from file''' 
        b=None
        with open(filename, 'rb') as f:
            b=f.read()
        error=SyxPacked32Voice.corruptSyx_(b)
        if error is not None:
            filename=os.path.basename(filename)
            dialogManager.showErrorDialog("Load SysEx (.syx) file",
                                          filename+" is corrupt",
                                          error)
            return None
        return SyxPacked32Voice(b)

    class VoiceData:
        def __init__(self, data):
            self.rawSyxData=data

        def getName(self):
            return self.rawSyxData[118:128].decode().rstrip()

        def __getitem__(self, index):
            return self.rawSyxData.__getitem__(index)
