/*

The classes and methods in this file are NOT a public API and are subject to change!

*/

UGenRegressionTest {

    // Run a SynthDef, record its output to a file in NRT mode, and then write to file.
    // Duration is in samples, not seconds, so it has no dependency on options.
    *recordSynthDefNRT { |ugenGraphFunc, options, duration = 100, outFile, action|
        var synthDefName, scoreFile;
        var score;

        // Filesystem clutter is avoided in two ways:
        // 1. Every generated SynthDef has the same name, so they overwrite each other.
        // 2. The SynthDef has the "temp__" prefix, so sclang will delete it on next boot.
        synthDefName = SystemSynthDefs.tempNamePrefix ++ "0";

        // This is an undocumented method used in Function:play. It wraps a UGen graph
        // function in an Out UGen if there isn't one present.
        GraphBuilder.wrapOut(
            synthDefName,
            ugenGraphFunc,
            fadeTime: nil
        ).writeDefFile;

        options = options ?? {
            ServerOptions()
                .sampleRate_(48000)
                .numOutputBusChannels_(1);
        };

        score = [
            [0, [\s_new, synthDefName, 1000, 0, 0]],
            [duration / options.sampleRate, [0]]
        ];

        scoreFile = Platform.defaultTempDir +/+ "SuperCollider_UGenRegressionTest_score.osc";

        Score.recordNRT(
            score,
            scoreFile,
            outFile,
            sampleRate: options.sampleRate,
            options: options,
            action: action
        );
    }

    *loadToFloatArrayNRT { |ugenGraphFunc, options, duration = 100, action|
        var outFile;
        outFile = Platform.defaultTempDir +/+ "SuperCollider_UGenRegressionTest_output.aiff";

        // Remove any prior output file (so if the file isn't generated, we know about it).
        File.delete(outFile);

        this.recordSynthDefNRT(
            ugenGraphFunc: ugenGraphFunc,
            duration: duration,
            options: options,
            outFile: outFile,
            action: { |scsynthExitCode|
                var soundFile, signal;
                if(scsynthExitCode != 0) {
                    action.(nil);
                } {
                    soundFile = SoundFile.openRead(outFile);
                    signal = FloatArray.newClear(soundFile.numFrames);
                    soundFile.readData(signal);
                    soundFile.close;
                    action.(signal);
                };
            }
        );
    }

    // Convert FloatArray[0.1, 0.2] to "3DCCCCCD 3E4CCCCD"
    *signalAsByteString { |signal|
        ^signal.collect { |x| x.as32Bits.asHexString }.join($ );
    }

    *generateTestCode { |functionCode, options, action|
        var ugenGraphFunc;
        ugenGraphFunc = functionCode.interpret;
        options = options ?? {
            ServerOptions()
                .sampleRate_(48000)
                .numOutputBusChannels_(1);
        };
        this.loadToFloatArrayNRT(
            ugenGraphFunc: ugenGraphFunc,
            options: options,
            action: { |signal|
                var code;
                code = [
                    "code:" + functionCode,
                    "sampleRate:" + options.sampleRate,
                    "blockSize:" + options.blockSize,
                    "numOutputBusChannels:" + options.numOutputBusChannels,
                    "signal:" + this.signalAsByteString(signal)
                ].join($\n);
                action.(code);
            }
        );
    }

}
