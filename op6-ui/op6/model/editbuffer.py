from math import log2, log10, frexp

def _checkIntParam(value, maxValue=99, minValue=0):
    try:
        value=max(minValue, min(int(value), maxValue))
    except ValueError:
        value=None
    return value

def _checkBoolParam(value):
    return _checkIntParam(value, maxValue=1)

def _checkSingleDigitParam(value):
    return _checkIntParam(value, maxValue=9)

def _checkPmLevelParam(value):
    return _checkIntParam(value, minValue=-99)

def _checkFreqParam(value, maxValue):
    try:
        value=min(float(value), maxValue)
    except ValueError:
        value=None
    return value

def _checkStringParam(value):
    return str(value)

_opParameters={
    "Envelope Time 1": (0, _checkIntParam),
    "Envelope Time 2": (1, _checkIntParam),
    "Envelope Time 3": (2, _checkIntParam),
    "Envelope Time 4": (3, _checkIntParam),
    "Envelope Level 0": (4, _checkIntParam),
    "Envelope Level 1": (5, _checkIntParam),
    "Envelope Level 2": (6, _checkIntParam),
    "Envelope Level 3": (7, _checkIntParam),
    "Envelope Level 4": (8, _checkIntParam),
    "Keyboard Level Scaling Breakpoint": (9, lambda x: _checkIntParam(x, 127)),
    "Keyboard Level Scaling Left Depth": (10, _checkIntParam),
    "Keyboard Level Scaling Left Curve": (11, lambda x: _checkIntParam(x, 3)),
    "Keyboard Level Scaling Right Depth": (12, _checkIntParam),
    "Keyboard Level Scaling Right Curve": (13, lambda x: _checkIntParam(x, 3)),
    "Frequency Mode": (14, _checkBoolParam),
    "Frequency": (15, _checkFreqParam),
    "Total Output Level": (16, _checkIntParam),
    "Amplitude Modulation Sensitivity": (17, lambda x: _checkIntParam(x, 3)),
    "Velocity Sensitivity": (18, lambda x: _checkIntParam(x, 7)),
    "Keyboard Rate Scaling": (19, lambda x: _checkIntParam(x, 7)),
    "Operator Enable": (20, _checkBoolParam)
}

_paramsPerOp=21
_firstCommon=128

_commonParameters={
    "Pitch Envelope Time 1": (_firstCommon, _checkIntParam),
    "Pitch Envelope Time 2": (_firstCommon+1, _checkIntParam),
    "Pitch Envelope Time 3": (_firstCommon+2, _checkIntParam),
    "Pitch Envelope Time 4": (_firstCommon+3, _checkIntParam),
    "Pitch Envelope Level 0": (_firstCommon+4, _checkPmLevelParam),
    "Pitch Envelope Level 1": (_firstCommon+5, _checkPmLevelParam),
    "Pitch Envelope Level 2": (_firstCommon+6, _checkPmLevelParam),
    "Pitch Envelope Level 3": (_firstCommon+7, _checkPmLevelParam),
    "Pitch Envelope Level 4": (_firstCommon+8, _checkPmLevelParam),
    "Algorithm": (_firstCommon+9, lambda x: _checkIntParam(x, 31)),
    "Feedback":  (_firstCommon+10, _checkSingleDigitParam),
#    "Oscillator Sync": (_firstCommon+11, _checkBoolParam),
#    "Pitch Envelope Depth": (_firstCommon+12, lambda x: _checkIntParam(x, 12)),
    "Pitch Modulation Sensitivity": (_firstCommon+13, lambda x: _checkIntParam(x, 7)),
#    "Velocity Sensitivity": (_firstCommon+14, _checkSingleDigitParam),
#    "Keyboard Rate Scaling": (_firstCommon+15, _checkSingleDigitParam),
    "LFO Speed": (_firstCommon+16, _checkIntParam),
    "LFO Delay": (_firstCommon+17, _checkIntParam),
    "LFO Waveform": (_firstCommon+18, lambda x: _checkIntParam(x, 5)),
    "LFO Initial Pitch Modulation Depth": (_firstCommon+20, _checkIntParam),
    "LFO Initial Amplitude Modulation Depth": (_firstCommon+21, _checkIntParam),
    "Voice Name": (_firstCommon+22, _checkStringParam),
}

_lastCommon=_firstCommon+22

class EditBuffer:
    def __init__(self):
        self.parameters=None
        self.initialVoice_=None
        self.setInitialVoice()

    def getVoiceParameters(self):
        return tuple(self.parameters)

    def setVoiceParametersUnchecked(self, parameters):
        self.parameters=list(parameters)

    def _checkParameter(self, value, checker, page):
        if checker==_checkFreqParam:
            d0=page*_paramsPerOp
            freqMode=self._getOpParameter(d0, "Frequency Mode")
            maxValue=9999.9 if freqMode==1 else 99.999
            return _checkFreqParam(value, maxValue)
        else:
            return checker(value)
        
    def setVoiceParametersChecked(self, parameters):
        for op in range(6):
            d0=(5-op)*_paramsPerOp
            for (name, (index, checker)) in _opParameters.items():
                value=parameters[d0+index]
                value=self._checkParameter(value, checker, op)
                if value is not None:
                    self.parameters[d0+index]=value
        COMMON_PAGE=6
        for (name, (index, checker)) in _commonParameters.items():
            value=parameters[index]
            value=self._checkParameter(value, checker, COMMON_PAGE)
            if value is not None:
                self.parameters[index]=value

    def setInitialVoice(self):
        if self.initialVoice_ is not None:
            self.parameters=list(self.initialVoice_)
            return

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
            # Operator Enable
            self._setOpParameter(d0, "Operator Enable", 1)

        # Unused parameter numbers
        for i in range(6*_paramsPerOp, _firstCommon):
            self.parameters[i]=None
        # Common Voice Parameters
#        self._setCommonParameter("Pitch Envelope Depth", 12)
        self._setCommonParameter("LFO Speed", 35)
        self._setCommonParameter("Voice Name", "INIT VOICE")

        # Cache the initial voice
        self.initialVoice_=tuple(self.parameters)
    
    def loadFromSyx(self, syxVoiceData):
        self.setInitialVoice()
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
        self._setOpParameter(d0, "Keyboard Level Scaling Breakpoint",
                             max(min(syxOpData[8]+60-39,127),0))
        # curves: -LIN (0) -EXP (1) +EXP(2) +LIN(3)
        RC_LC=syxOpData[11]
        lDepth=min(syxOpData[9], 99)
        lCurve=RC_LC & 3
        self._setOpParameter(d0, "Keyboard Level Scaling Left Depth", lDepth)
        self._setOpParameter(d0, "Keyboard Level Scaling Left Curve", lCurve)
        rDepth=min(syxOpData[10], 99)
        rCurve=(RC_LC >> 2) & 3
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
#        self._setCommonParameter("Oscillator Sync", (OPI_FB & 8)>>3)
        LPMS_LFW_LFKS=syxVoiceData[116]
        self._setCommonParameter("Pitch Modulation Sensitivity",
                                 (LPMS_LFW_LFKS>>4) & 7)
#        self._setCommonParameter("Pitch Envelope Depth", 12)
#        self._setCommonParameter("Velocity Sensitivity", 0)
        # ...and don't forget (pitch) "Keyboard Rate Scaling" (wasn't here)
        # LFO
        self._setCommonParameter("LFO Speed", min(syxVoiceData[112],99))
        self._setCommonParameter("LFO Delay", min(syxVoiceData[113],99))
        self._setCommonParameter("LFO Waveform", min((LPMS_LFW_LFKS>>1) & 7, 5))
        self._setCommonParameter("LFO Initial Pitch Modulation Depth",
                                 min(syxVoiceData[114],99))
        self._setCommonParameter("LFO Initial Amplitude Modulation Depth",
                                 min(syxVoiceData[115],99))
        # ...and the name
        self._setCommonParameter("Voice Name", syxVoiceData.getName())

    def _getOpParameter(self, d0, paramName):
        (index, _)=_opParameters[paramName]
        return self.parameters[d0+index]

    def _setOpParameter(self, d0, paramName, value):
        (index, _)=_opParameters[paramName]
        self.parameters[d0+index]=value

    def _setCommonParameter(self, paramName, value):
        (index, _)=_commonParameters[paramName]
        self.parameters[index]=value
  
    def _getParameterPage(self, paramName):
        '''Parameter Page: Op6 (0), Op5 (1), ..., Op1 (5), Common (6)'''
        return (6-int(paramName[2])) if paramName.startswith("Op") else 6

    def _getParameterTuple(self, paramName):
        '''returns (index, parameter type, midi nrpn)'''
        page=self._getParameterPage(paramName)
        (parameters, key)=((_commonParameters, paramName) if page==6 else
                           (_opParameters, paramName[4:]))
        t=parameters[key]
        index=t[0] if page==6 else t[0] + page*_paramsPerOp
        lsb=t[0]-_firstCommon if page==6 else t[0]
        return (index, t[1], 128*page+lsb)

    def setVoiceParameter(self, paramName, paramValue):
        (paramIndex, checker, nrpn) = self._getParameterTuple(paramName)
        page=nrpn>>7
        paramValue=self._checkParameter(paramValue, checker, page)
        if paramValue is None:
            return False

        if self.parameters[paramIndex]!=paramValue:
            self.parameters[paramIndex]=paramValue
            return True
        else:
            return False

    def getVoiceParameter(self, paramName):
        (paramIndex, *_) = self._getParameterTuple(paramName)
        return self.parameters[paramIndex]

    def getAllVoiceParameters(self, skipInitialValue=False):
        '''aggregate all voice parameters (name, value) in a list'''
        result=[]
        for op in range(6):
            i0=(5-op)*_paramsPerOp
            prefix="Op"+str(op+1)+" "
            for (name, (index,_)) in _opParameters.items():
                value=self.parameters[i0+index]
                if not skipInitialValue or value!=self.initialVoice_[i0+index]:
                    result.append((prefix+name, value))
        for (name, (index,_)) in _commonParameters.items():
            value=self.parameters[index]
            if not skipInitialValue or value!=self.initialVoice_[index]:
                result.append((name, value))
        return result

    def _repackFrequencyParameters(self, op):
        FIXED_FREQUENCY=14
        FREQUENCY_RATIO=15
        d0=op*_paramsPerOp
        freqMode=self._getOpParameter(d0, "Frequency Mode");
        freq=self._getOpParameter(d0, "Frequency");
        # The float value is represented with 3 bits of exponent
        # and 1+11 bits of mantissa (with an implicit msb).
        # Range for ratios [0, 64) fixed frequencies [0, 16kHz]
        (nrpn, expBias)=((FREQUENCY_RATIO, 0) if freqMode==0 else
                         (FIXED_FREQUENCY, 7))
        (f, e)=frexp(freq)
        e-=expBias
        m=round(f*0x1000)
        if m==0x1000:
            # rounded to next exponent
            e+=1
            m>>=1
        if e>7:
            # max value
            e=7
            m=0x7ff
        elif e<=0:
            # gradual/underflow
            m>>=1-e
            e=0
        return (op*128+nrpn, (e<<11)|(m & 0x7ff))

    def sendVoiceParameter(self, paramName, midiOut, channel):
        (index, _, nrpn) = self._getParameterTuple(paramName)
        # TODO: this conversion needs to be done on a per-parameter basis
        # this is quickly becoming messy
        paramValue=self.parameters[index]
        x=128*paramValue
        if nrpn < 6*128:
            # OP(n)
            PARAM_FREQUENCY_MODE=14
            PARAM_FREQUENCY=15
            lsb=(nrpn & 127)
            if lsb==PARAM_FREQUENCY_MODE or lsb==PARAM_FREQUENCY:
                # Frequency Mode and Frequency are repackaged
                # as FixedFrequency and FrequencyRatio parameters
                (newNRPN, value)=self._repackFrequencyParameters(nrpn>>7)
                nrpn=newNRPN # FixedFreq or FreqRatio
                x=value
        elif 6*128+4 <= nrpn <= 6*128+8:
            # Pitch envelope levels (-99..+99)
            x=(64*paramValue) & 0x3fff
            
        # TODO: check if we have an active midi connection
        midiOut.sendParameter(channel, nrpn, x)

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
