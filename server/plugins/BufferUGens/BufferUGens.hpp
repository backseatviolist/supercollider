#include "SC_PlugIn.h"

static InterfaceTable* ft;

inline double sc_loop(Unit *unit, double in, double hi, int loop)
{
    // avoid the divide if possible
    if (in >= hi) {
        if (!loop) {
            unit->mDone = true;
            return hi;
        }
        in -= hi;
        if (in < hi) return in;
    } else if (in < 0.) {
        if (!loop) {
            unit->mDone = true;
            return 0.;
        }
        in += hi;
        if (in >= 0.) return in;
    } else return in;

    return in - hi * floor(in/hi);
}

#define LOOP_BODY_1(SAMPLE_INDEX) \
        phase = sc_loop((Unit*)unit, phase, loopMax, loop); \
        int32 iphase = (int32)phase; \
        const float* table1 = bufData + iphase * bufChannels; \
        int32 index = 0; \
        if(numOutputs == bufChannels) { \
            for (uint32 channel=0; channel<numOutputs; ++channel) { \
                LOOP_INNER_BODY_1(SAMPLE_INDEX) \
                index++; \
            } \
        } else if (numOutputs < bufChannels) { \
            for (uint32 channel=0; channel<numOutputs; ++channel) { \
                LOOP_INNER_BODY_1(SAMPLE_INDEX) \
                index++; \
            } \
            index += (bufChannels - numOutputs); \
        } else { \
            for (uint32 channel=0; channel<bufChannels; ++channel) { \
                LOOP_INNER_BODY_1(SAMPLE_INDEX) \
                index++; \
            } \
            for (uint32 channel=bufChannels; channel<numOutputs; ++channel) { \
                OUT(channel)[SAMPLE_INDEX] = 0.f; \
                index++; \
            } \
        } \

#define LOOP_BODY_2(SAMPLE_INDEX) \
        phase = sc_loop((Unit*)unit, phase, loopMax, loop); \
        int32 iphase = (int32)phase; \
        const float* table1 = bufData + iphase * bufChannels; \
        const float* table2 = table1 + bufChannels; \
        if (iphase > guardFrame) { \
            if (loop) { \
                table2 -= bufSamples; \
            } else { \
                table2 -= bufChannels; \
            } \
        } \
        int32 index = 0; \
        float fracphase = phase - (double)iphase; \
        if(numOutputs == bufChannels) { \
            for (uint32 channel=0; channel<numOutputs; ++channel) { \
                LOOP_INNER_BODY_2(SAMPLE_INDEX) \
                index++; \
            } \
        } else if (numOutputs < bufChannels) { \
            for (uint32 channel=0; channel<numOutputs; ++channel) { \
                LOOP_INNER_BODY_2(SAMPLE_INDEX) \
                index++; \
            } \
            index += (bufChannels - numOutputs); \
        } else { \
            for (uint32 channel=0; channel<bufChannels; ++channel) { \
                LOOP_INNER_BODY_2(SAMPLE_INDEX) \
                index++; \
            } \
            for (uint32 channel=bufChannels; channel<numOutputs; ++channel) { \
                OUT(channel)[SAMPLE_INDEX] = 0.f; \
                index++; \
            } \
        } \

#define LOOP_BODY_4(SAMPLE_INDEX) \
        phase = sc_loop((Unit*)unit, phase, loopMax, loop); \
        int32 iphase = (int32)phase; \
        const float* table1 = bufData + iphase * bufChannels; \
        const float* table0 = table1 - bufChannels; \
        const float* table2 = table1 + bufChannels; \
        const float* table3 = table2 + bufChannels; \
        if (iphase == 0) { \
            if (loop) { \
                table0 += bufSamples; \
            } else { \
                table0 += bufChannels; \
            } \
        } else if (iphase >= guardFrame) { \
            if (iphase == guardFrame) { \
                if (loop) { \
                    table3 -= bufSamples; \
                } else { \
                    table3 -= bufChannels; \
                } \
            } else { \
                if (loop) { \
                    table2 -= bufSamples; \
                    table3 -= bufSamples; \
                } else { \
                    table2 -= bufChannels; \
                    table3 -= 2 * bufChannels; \
                } \
            } \
        } \
        int32 index = 0; \
        float fracphase = phase - (double)iphase; \
        if(numOutputs == bufChannels) { \
            for (uint32 channel=0; channel<numOutputs; ++channel) { \
                LOOP_INNER_BODY_4(SAMPLE_INDEX) \
                index++; \
            } \
        } else if (numOutputs < bufChannels) { \
            for (uint32 channel=0; channel<numOutputs; ++channel) { \
                LOOP_INNER_BODY_4(SAMPLE_INDEX) \
                index++; \
            } \
            index += (bufChannels - numOutputs); \
        } else { \
            for (uint32 channel=0; channel<bufChannels; ++channel) { \
                LOOP_INNER_BODY_4(SAMPLE_INDEX) \
                index++; \
            } \
            for (uint32 channel=bufChannels; channel<numOutputs; ++channel) { \
                OUT(channel)[SAMPLE_INDEX] = 0.f; \
                index++; \
            } \
        } \

#define LOOP_INNER_BODY_1(SAMPLE_INDEX) \
    OUT(channel)[SAMPLE_INDEX] = table1[index]; \

#define LOOP_INNER_BODY_2(SAMPLE_INDEX) \
    float b = table1[index]; \
    float c = table2[index]; \
    OUT(channel)[SAMPLE_INDEX] = b + fracphase * (c - b); \

#define LOOP_INNER_BODY_4(SAMPLE_INDEX) \
    float a = table0[index]; \
    float b = table1[index]; \
    float c = table2[index]; \
    float d = table3[index]; \
    OUT(channel)[SAMPLE_INDEX] = cubicinterp(fracphase, a, b, c, d); \

#define CHECK_BUFFER_DATA \
if (!bufData) { \
    if(unit->mWorld->mVerbosity > -1 && !unit->mDone && (unit->m_failedBufNum != fbufnum)) { \
        Print("Buffer UGen: no buffer data\n"); \
        unit->m_failedBufNum = fbufnum; \
    } \
    ClearUnitOutputs(unit, inNumSamples); \
    return; \
} else { \
    if (bufChannels != numOutputs) { \
        if(unit->mWorld->mVerbosity > -1 && !unit->mDone && (unit->m_failedBufNum != fbufnum)) { \
            Print("Buffer UGen channel mismatch: expected %i, yet buffer has %i channels\n", \
                  numOutputs, bufChannels); \
            unit->m_failedBufNum = fbufnum; \
            } \
        } \
} \

