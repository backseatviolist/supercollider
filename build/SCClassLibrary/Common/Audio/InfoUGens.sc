
InfoUGenBase : UGen {
	*ir {
		^this.multiNew('scalar')
	}
}

BufInfoUGenBase : InfoUGenBase {
	*kr { arg bufnum;
		^this.multiNew('control', bufnum)
	}
	*ir { arg bufnum;
		^this.multiNew('scalar',bufnum)
	}
}

SampleRate : InfoUGenBase {}
SampleDur : InfoUGenBase {}
RadiansPerSample : InfoUGenBase {}
ControlRate : InfoUGenBase {}

NumOutputBuses : InfoUGenBase {}
NumInputBuses : InfoUGenBase {}
NumAudioBuses : InfoUGenBase {}
NumControlBuses : InfoUGenBase {}
NumBuffers : InfoUGenBase {}
NumRunningSynths : InfoUGenBase {}

BufSampleRate : BufInfoUGenBase {}
BufRateScale : BufInfoUGenBase {}
BufFrames : BufInfoUGenBase {}
BufSamples : BufInfoUGenBase {}
BufDur : BufInfoUGenBase {}
BufChannels : BufInfoUGenBase {}

////////////////////////////////////////////

