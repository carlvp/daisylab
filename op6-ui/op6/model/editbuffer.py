IntParamType=1
FpParamType=2
StringParam=3

_opParameters={
    "Envelope Time 1": (0, IntParamType),
    "Envelope Time 2": (1, IntParamType),
    "Envelope Time 3": (2, IntParamType),
    "Envelope Time 4": (3, IntParamType),
    "Envelope Level 0": (4, IntParamType),
    "Envelope Level 1": (5, IntParamType),
    "Envelope Level 2": (6, IntParamType),
    "Envelope Level 3": (7, IntParamType),
    "Envelope Level 4": (8, IntParamType),
    "Keyboard Level Scaling Breakpoint": (9, IntParamType),
    "Keyboard Level Scaling Left Depth": (10, IntParamType),
    "Keyboard Level Scaling Left Curve": (11, IntParamType),
    "Keyboard Level Scaling Right Depth": (12, IntParamType),
    "Keyboard Level Scaling Right Curve": (13, IntParamType),
    "Frequency Mode": (14, IntParamType),
    "Frequency": (15, FpParamType),
    "Total Output Level": (16, IntParamType),
    "Amplitude Modulation Sensitivity": (17, IntParamType),
    "Velocity Sensitivity": (18, IntParamType),
    "Keyboard Rate Scaling": (19, IntParamType),
#    "Operator Enable": (20, IntParamType)
}

_paramsPerOp=21
_firstCommon=128

_commonParameters={
    "Pitch Envelope Time 1": (_firstCommon, IntParamType),
    "Pitch Envelope Time 2": (_firstCommon+1, IntParamType),
    "Pitch Envelope Time 3": (_firstCommon+2, IntParamType),
    "Pitch Envelope Time 4": (_firstCommon+3, IntParamType),
    "Pitch Envelope Level 0": (_firstCommon+4, IntParamType),
    "Pitch Envelope Level 1": (_firstCommon+5, IntParamType),
    "Pitch Envelope Level 2": (_firstCommon+6, IntParamType),
    "Pitch Envelope Level 3": (_firstCommon+7, IntParamType),
    "Pitch Envelope Level 4": (_firstCommon+8, IntParamType),
    "Algorithm": (_firstCommon+9, IntParamType),
    "Feedback":  (_firstCommon+10, IntParamType),
    "Oscillator Sync": (_firstCommon+11, IntParamType),
    "Pitch Envelope Depth": (_firstCommon+12, IntParamType),
    "Pitch Modulation Sensitivity": (_firstCommon+13, IntParamType),
    "Velocity Sensitivity": (_firstCommon+14, IntParamType),
    "Keyboard Rate Scaling": (_firstCommon+15, IntParamType),
    "LFO Speed": (_firstCommon+16, IntParamType),
    "LFO Delay": (_firstCommon+17, IntParamType),
    "LFO Sync": (_firstCommon+18, IntParamType),
    "LFO Waveform": (_firstCommon+19, IntParamType),
    "LFO Initial Pitch Modulation Depth": (_firstCommon+20, IntParamType),
    "LFO Initial Amplitude Modulation Depth": (_firstCommon+21, IntParamType),
    "Voice Name": (_firstCommon+22, IntParamType),
}

_lastCommon=_firstCommon+22

class EditBuffer:
    def __init__(self):
        self.setInitialVoice()

    def loadFromSyx(self, syxVoiceData):
        pass

    def setInitialVoice(self):
        self.parameters=[0 for param in range(_lastCommon+1)]
        for op in range(6):
            i=op*_paramsPerOp
            # Envelope Levels L1, L2, L3
            self.parameters[i+5]=99
            self.parameters[i+6]=99
            self.parameters[i+7]=99
            # Breakpoint
            self.parameters[i+9]=60
            # Frequency
            self.parameters[i+15]=1.0
            # Output Level (op1)
            if op==5:
                self.parameters[i+16]=99
        # Unused parameter numbers
        for i in range(6*_paramsPerOp, _firstCommon):
            self.parameters[i]=None
        # PEG Depth
        self.parameters[_firstCommon+12]=12
        # LFO Speed
        self.parameters[_firstCommon+16]=35
        # Voice Name
        self.parameters[_firstCommon+22]="INIT VOICE"

    def _getParameterTuple(self, paramName):
        firstParam=0
        parameters=_commonParameters
        if paramName.startswith("Op"):
            op=int(paramName[2])
            firstParam=(6-op)*_paramsPerOp
            paramName=paramName[4:]
            parameters=_opParameters
        t=parameters[paramName]
        return (t[0]+firstParam, t[1])

    def setVoiceParameter(self, paramName, paramValue):
        pass

    def getVoiceParameter(self, paramName):
        (paramIndex, _) = self._getParameterTuple(paramName)
        return self.parameters[paramIndex]
        
    def getAllVoiceParameters(self):
        '''aggregate all voice paramters (name, value) in a list'''
        result=[]
        for op in range(6):
            i0=(5-op)*_paramsPerOp
            prefix="Op"+str(op+1)+" "
            for (name, (index,_)) in _opParameters.items():
                value=self.parameters[i0+index]
                result.append((prefix+name, value))
        for (name, (index,_)) in _commonParameters.items():
            value=self.parameters[index]
            result.append((name, value))
        return result
