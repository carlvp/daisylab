import json
import os
import os.path
from .editbuffer import EditBuffer, VOICE_NAME_INDEX

PROGRAMS_PER_CARTRIDGE=32

def mkdir_p_(path):
    if not os.path.exists(path):
        (head, tail)=os.path.split(path)
        if head!='':
            mkdir_p_(head)
        os.mkdir(path)

class ProgramBank:
    '''
    The ProgramBank is a repository for voices/programs/patches, well
    the parameters that specify sounds. It manages persistant storage
    of the programs.
    '''

    def __init__(self, numPrograms, pathToStorage):
        self.program=[None]*numPrograms
        self.pathToStorage=pathToStorage

    def getProgram(self, index):
        return self.program[index]

    def setProgram(self, index, program):
        self.program[index]=program

    def getProgramName(self, index):
        pgm=self.program[index]
        return "" if pgm is None else pgm[VOICE_NAME_INDEX]

    def getBankLetter(self, index):
        return chr(65+index//32) # bank A, B, C,...

    def getOffsetInBank(self, index):
        return 1+(index & 31)    # program 1, 2,...,32 (within a bank A, B, C)

    def getProgramPath_(self, index, createDirectory=False):
        bank=self.getBankLetter(index)
        prog=self.getOffsetInBank(index)
        directory=os.path.join(self.pathToStorage, bank)
        filename=f'{prog:02d}.json'
        if createDirectory:
            mkdir_p_(directory)
        return os.path.join(directory, filename)

    def saveEditBuffer(self, index, editBuffer):
        '''saves the editBuffer to program bank and file'''
        program=editBuffer.asProgram()
        self.setProgram(index, program)

        # commit to file
        filename=self.getProgramPath_(index, createDirectory=True)
        with open(filename, 'w') as file:
            file.write('{\n')
            name=editBuffer.getVoiceParameter("Voice Name")
            file.write(f'  "Voice Name": "{name}"')

            items=editBuffer.getAllVoiceParameters(skipInitialValue=True)
            for (param, value) in items:
                if param!="Voice Name":
                    file.write(f',\n  "{param}": {value}')
            file.write('\n}\n')
        return False # success

    def loadVoiceParameters_(self, filename):
        '''loads a program from persistent storage
           returns a map of {parameter-name: value} items'''
        with open(filename, 'r') as file:
            data=file.read()
        return json.loads(data)

    def importProgram_(self, fileName, editBuffer):
        '''loads a program from persistent storage'''
        voiceMap=self.loadVoiceParameters_(fileName)
        # apply parameters to editBuffer according to saved programs
        editBuffer.setInitialVoice()
        for (paramName, value) in voiceMap.items():
            editBuffer.setVoiceParameter(paramName, value)

    def importPrograms(self):
        '''loads all programs from persistent storage'''
        numPrograms=len(self.program)
        editBuffer=EditBuffer()
        for p0 in range(0, numPrograms, 32):
            # The programs are organized in directories A, B, C,...
            # TODO: this can be simplified by not exposing loadProgram
            if os.path.isdir(os.path.join(self.pathToStorage,
                                          self.getBankLetter(p0))):
                # Check the presence of files named 01.json,..., 32.json
                end=min(p0+32, numPrograms)
                for index in range(p0, end):
                    fileName=self.getProgramPath_(index)
                    if os.path.isfile(fileName):
                        self.importProgram_(fileName, editBuffer)
                        program=editBuffer.asProgram()
                        self.setProgram(index, program)
