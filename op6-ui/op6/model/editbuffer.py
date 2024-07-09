from math import log2, log10

IntParamType=1
FpParamType=2
StringParamType=3

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
    "Voice Name": (_firstCommon+22, StringParamType),
}

_lastCommon=_firstCommon+22


def _checkParam(paramValue, paramType):
    if paramType==IntParamType:
        try:
            paramValue=int(paramValue)
        except ValueError:
            paramValue=None
    elif paramType==FpParamType:
        try:
            paramValue=float(paramValue)
        except ValueError:
            paramValue=None
    else:
        assert paramType==StringParamType
        paramValue=str(paramValue)
    return paramValue

class EditBuffer:
    def __init__(self):
        self.parameters=None
        self.setInitialVoice()

    def getVoiceParameters():
        return tuple(self.parameters)

    def setVoiceParameters(parameters):
        for op in range(6):
            d0=(5-op)*_paramsPerOp
            for (name, (index, paramType)) in _opParameters.items():
                value=parameters[d0+index]
                value=_checkParam(value, paramType)
                if value is not None:
                    self.parameters[d0+index]=value
        for (name, (index, paramType)) in _commonParameters.items():
            value=parameters[index]
            value=_checkParam(value, paramType)
            if value is not None:
                self.parameters[index]=value

    def setInitialVoice(self):
        self.parameters=[0 for param in range(_lastCommon+1)]
        for op in range(6):
            d0=op*_paramsPerOp
            # Envelope
            self._setOpParameter(d0, "Envelope Level 1", 99)
            self._setOpParameter(d0, "Envelope Level 2", 99)
            self._setOpParameter(d0, "Envelope Level 3", 99)
            # Breakpoint
            self._setOpParameter(d0, "Keyboard Level Scaling Breakpoint", 60)
            # Frequency
            self._setOpParameter(d0, "Frequency", 1.0)
            # Output Level (op1)
            if op==5:
                self._setOpParameter(d0, "Total Output Level", 99)
        # Unused parameter numbers
        for i in range(6*_paramsPerOp, _firstCommon):
            self.parameters[i]=None
        # Common Voice Parameters
        self._setCommonParameter("Pitch Envelope Depth", 12)
        self._setCommonParameter("LFO Speed", 35)
        self._setCommonParameter("Voice Name", "INIT VOICE")
    
    def loadFromSyx(self, syxVoiceData):
        for op in range(6):
            s0=17*op
            d0=_paramsPerOp*op
            self._loadOpParametersFromSyx(d0, syxVoiceData[s0:s0+17])
        self._loadCommonParametersFromSyx(syxVoiceData)

    def _loadOpParametersFromSyx(self, d0, syxOpData):
        # Envelope times
        (t1, t2, t3, t4)=_convertEnvelopeTimes(syxOpData)
        self._setOpParameter(d0, "Envelope Time 1", t1)
        self._setOpParameter(d0, "Envelope Time 2", t2)
        self._setOpParameter(d0, "Envelope Time 3", t3)
        self._setOpParameter(d0, "Envelope Time 4", t4)
        # Envelope levels
        L0_L4=min(syxOpData[7], 99)
        self._setOpParameter(d0, "Envelope Level 0", L0_L4)
        self._setOpParameter(d0, "Envelope Level 1", min(syxOpData[4], 99))
        self._setOpParameter(d0, "Envelope Level 2", min(syxOpData[5], 99))
        self._setOpParameter(d0, "Envelope Level 3", min(syxOpData[6], 99))
        self._setOpParameter(d0, "Envelope Level 4", L0_L4)
        # Keyboard level scaling
        def depthAndCurve(syxDepth, syxCurve):
            # curves: -LIN (0) -EXP (1) +EXP(2) +LIN(3)
            return (syxDepth if syxCurve>=2 else -syxDepth,
                    1 if syxCurve==1 or syxCurve==2 else 0)

        self._setOpParameter(d0, "Keyboard Level Scaling Breakpoint",
                             max(min(syxOpData[8]+60-39,127),0))
        RC_LC=syxOpData[11]
        (lDepth, lCurve)=depthAndCurve(min(syxOpData[9], 99), RC_LC & 3)
        self._setOpParameter(d0, "Keyboard Level Scaling Left Depth", lDepth)
        self._setOpParameter(d0, "Keyboard Level Scaling Left Curve", lCurve)
        (rDepth, rCurve)=depthAndCurve(min(syxOpData[10], 99), (RC_LC >> 2) & 3)
        self._setOpParameter(d0, "Keyboard Level Scaling Right Depth", rDepth)
        self._setOpParameter(d0, "Keyboard Level Scaling Right Curve", rCurve)
        # Other stuff
        PD_RS=syxOpData[12]
        self._setOpParameter(d0, "Keyboard Rate Scaling", PD_RS & 7)
        TS_AMS=syxOpData[13]
        self._setOpParameter(d0, "Amplitude Modulation Sensitivity", TS_AMS & 3)
        self._setOpParameter(d0, "Velocity Sensitivity", (TS_AMS >> 2) & 7)
        self._setOpParameter(d0, "Total Output Level", min(syxOpData[14], 99))
        # Frequency
        def computeFrequency(coarse, fine, detune, fixed):
            f = (10**((coarse & 3) + fine*0.01) if fixed
                 else (0.5 if coarse==0 else coarse)*(1+fine*0.01))
            # detune=7 is zero cents, <7 means negative, >7 positive
            DETUNE=(0.9765957, 0.9822531, 0.9833885, 0.9868025,
                    0.9902285, 0.9936662, 0.9971160, 1.0000000,
	            1.0028923, 1.0063740, 1.0098680, 1.0133740,
                    1.0168921, 1.0180676, 1.0239652)
            return f*DETUNE[min(detune, 14)]

        PC_PM=syxOpData[15]
        self._setOpParameter(d0, "Frequency Mode", PC_PM & 1)
        self._setOpParameter(d0, "Frequency",
                             computeFrequency((PC_PM>>1) & 31,
                                              min(syxOpData[16], 99),
                                              (PD_RS>>3) & 15,
                                              (PC_PM & 1)!=0))

    def _loadCommonParametersFromSyx(self, syxVoiceData):
        # Envelope times
        (t1, t2, t3, t4)=_convertEnvelopeTimes(syxVoiceData[102:110])
        self._setCommonParameter("Pitch Envelope Time 1", t1)
        self._setCommonParameter("Pitch Envelope Time 2", t2)
        self._setCommonParameter("Pitch Envelope Time 3", t3)
        self._setCommonParameter("Pitch Envelope Time 4", t4)

        # Envelope levels, map 0->-99, 50->0 99->99
        def mapPegLevel(x):
            scale=99/50 if x<50 else 99/49
            return round((min(x,99)-50)*scale)

        L0_L4=mapPegLevel(syxVoiceData[109])
        self._setCommonParameter("Pitch Envelope Level 0", L0_L4)
        self._setCommonParameter("Pitch Envelope Level 1",
                                 mapPegLevel(syxVoiceData[106]))
        self._setCommonParameter("Pitch Envelope Level 2",
                                 mapPegLevel(syxVoiceData[107]))
        self._setCommonParameter("Pitch Envelope Level 3",
                                 mapPegLevel(syxVoiceData[108]))
        self._setCommonParameter("Pitch Envelope Level 4", L0_L4)
        # Other stuff
        self._setCommonParameter("Algorithm", syxVoiceData[110] & 31)
        OPI_FB=syxVoiceData[111]
        self._setCommonParameter("Feedback", OPI_FB & 7)
        self._setCommonParameter("Oscillator Sync", (OPI_FB & 8)>>3)
        LPMS_LFW_LFKS=syxVoiceData[116]
        self._setCommonParameter("Pitch Modulation Sensitivity",
                                 (LPMS_LFW_LFKS>>4) & 7)
        self._setCommonParameter("Pitch Envelope Depth", 12)
        self._setCommonParameter("Velocity Sensitivity", 0)
        # LFO
        self._setCommonParameter("LFO Speed", min(syxVoiceData[112],99))
        self._setCommonParameter("LFO Delay", min(syxVoiceData[113],99))
        self._setCommonParameter("LFO Sync", LPMS_LFW_LFKS & 1)
        self._setCommonParameter("LFO Waveform", min((LPMS_LFW_LFKS>>1) & 7, 5))
        self._setCommonParameter("LFO Initial Pitch Modulation Depth",
                                 min(syxVoiceData[114],99))
        self._setCommonParameter("LFO Initial Amplitude Modulation Depth",
                                 min(syxVoiceData[115],99))
        # ...and the name
        self._setCommonParameter("Voice Name", syxVoiceData.getName())

    def _setOpParameter(self, d0, paramName, value):
        (index, _)=_opParameters[paramName]
        self.parameters[d0+index]=value

    def _setCommonParameter(self, paramName, value):
        (index, _)=_commonParameters[paramName]
        self.parameters[index]=value

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
        (paramIndex, paramType) = self._getParameterTuple(paramName)
        paramValue=_checkParam(paramValue, paramType)
        if paramValue is None:
            return False

        if self.parameters[paramIndex]!=paramValue:
            self.parameters[paramIndex]=paramValue
            return True
        else:
            return False

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

# Conversion of envelope times

# Level, negative logcale 2^(-t/8), so unit is 1/8 octave
def _logLevel(tl99):
    first20=(127, 122, 118, 114, 110, 107, 104, 102, 100,  98,
             96,  94,  92,  90,  88,  86,  85,  84,  82,  81)
    return first20[tl99] if tl99<20 else 99-tl99

# Level in dB
def _level_dB(tl99):
    x=_logLevel(tl99)
    return -x*(20*log10(2)/8)

# rate in dB/s
def _rate_dBps(rate99):
    x=round(41*rate99/16)/16
    return 0.2819*(2**x)

MIN_ENVELOPE_TIME=0.004/3
MAX_ENVELOPE_TIME=20.032

# Time representation in logscale
def _timeInLogScale(t):
    # Look-up for small values
    # t99 setting:  0   1   2   3   4   5   6   7   8   9  10  11
    # num. blocks:  2   3   4   5   6   7   8   9  10  11  12  14
    # time [ms]:    1.3 2.0 2.7 3.3 4.0 4.7 5.3 6.0 6.7 7.3 8.0 9.3
    # index, i:     0   1   2   3   4   5   6   7   8   9  10  12
    i=max(0, round((t-MIN_ENVELOPE_TIME)*1500))
    return min(i,11) if i<=12 else (
           99 if t>MAX_ENVELOPE_TIME else
           99+round(8*log2(t/MAX_ENVELOPE_TIME)))
    # 99 ~ MAX_ENVELOPE_TIME=20.032
    # 91 ~ MAX_ENVELOPE_TIME/2=10.016, then 50% each 8 settings
    # ...
    # 12 ~ 10.7 ms (16 blocks)

# Compute envelope time such that it matches that of given rates/levels
# l1, l2 levels (0..99, in units of 1/8 octave)
# rate99 is the rate (0..99)
def _matchEnvelopeTime(rate99, l1, l2):
    dB1=_level_dB(l1)
    dB2=_level_dB(l2)
    dBps=_rate_dBps(rate99)
    t=abs(dB1-dB2)/dBps
    return _timeInLogScale(t)

# Compute envelope time such that the decay rate is matched
def _matchDecayRate(rate99):
    dBps=_rate_dBps(rate99)
    # "our" definition of envelope time is decay to exp(-pi)=0.0432 (-27 dB)
    t=27.287527/dBps
    return _timeInLogScale(t)

def _convertEnvelopeTimes(syxEnvParams):
    l0=min(max(syxEnvParams[7], 0), 99) # l0=l4
    l1=min(max(syxEnvParams[4], 0), 99)
    l2=min(max(syxEnvParams[5], 0), 99)
    # No perfect way of doing this...
    # Let the rate and the difference between levels determine first two
    # segments. In this way the envelope time is matched, makes sense for
    # attack.
    r1=min(max(syxEnvParams[0], 0), 99)
    t1=_matchEnvelopeTime(r1, l0, l1)
    r2=min(max(syxEnvParams[1], 0), 99)
    t2=_matchEnvelopeTime(r2, l1, l2)
    # Match the decay rate of the last two segments
    r3=min(max(syxEnvParams[2], 0), 99)
    t3=_matchDecayRate(r3)
    r4=min(max(syxEnvParams[3], 0), 99)
    t4=_matchDecayRate(r4)
    return (t1, t2, t3, t4)
