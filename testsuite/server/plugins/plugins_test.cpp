#include <iostream>
#include <boost/test/unit_test.hpp>
#include "SC_WorldOptions.h"
#include "SC_World.h"
#include "SC_Prototypes.h"
#include "SC_Errors.h"
#include "SC_Node.h"
#include "SC_Str4.h"
#include "SC_UnitDef.h"
#include "SC_Graph.h"
#include "SC_Wire.h"
#include "HashTable.h"

extern HashTable<struct UnitDef, Malloc> *gUnitDefLib;

class UGenRunner {
public:

    UGenRunner(double sample_rate, int buf_length) :
    m_sample_rate(sample_rate),
    m_buf_length(buf_length)
    {
        // Create world options.
        WorldOptions options = kDefaultWorldOptions;
        options.mBufLength = m_buf_length;
        options.mRealTime = false;
        options.mNumInputBusChannels = 1;
        options.mNumOutputBusChannels = 1;

        // Initialize the world object.
        m_world = World_New(&options);
        World_SetSampleRate(m_world, m_sample_rate);
        World_Start(m_world);
    }

    ~UGenRunner() {
        World_Cleanup(m_world, true);
    }

    std::vector<float> run_ugen(
        const char* name,
        std::vector<float> input_values
    ) {
        // Get the unit definition.
        UnitDef* unit_def = get_unit_def(name);

        const int num_inputs = input_values.size();
        const int num_outputs = 1;
        const int num_ports = num_inputs + num_outputs;

        // Mock a Graph object to serve as a parent for the unit.
        Graph* graph = (Graph*)malloc(sizeof(Graph));
        graph->mRGen = m_world->mRGen;

        // Create the unit.
        Unit* unit = (Unit*)malloc(unit_def->mAllocSize);
        unit->mWorld = m_world;
        unit->mParent = graph;
        unit->mUnitDef = unit_def;
        unit->mNumInputs = num_inputs;
        unit->mNumOutputs = num_outputs;

        unit->mInput = (Wire**)malloc(num_ports * sizeof(Wire*));
        unit->mOutput = unit->mInput + num_inputs;
        for (int i = 0; i < num_ports; i++) {
            unit->mInput[i] = (Wire*)malloc(sizeof(Wire));
            unit->mInput[i]->mCalcRate = calc_FullRate;
        }

        unit->mInBuf = (float**)malloc(num_ports * sizeof(float*));
        unit->mOutBuf = unit->mInBuf + num_inputs;
        for (int i = 0; i < num_ports; i++) {
            unit->mInBuf[i] = (float*)malloc(m_buf_length * sizeof(float));
        }

        unit->mCalcRate = calc_FullRate;
        unit->mSpecialIndex = 0; // ???

        unit->mRate = &m_world->mFullRate;
        unit->mBufLength = unit->mRate->mBufLength;

        unit->mDone = false;

        // Run the Ctor function.

        for (int i = 0; i < m_buf_length; i++) {
            for (int j = 0; j < num_inputs; j++) {
                unit->mInBuf[j][i] = input_values[j];
            }
            unit->mOutBuf[0][i] = 0;
        }
        (*unit->mUnitDef->mUnitCtorFunc)(unit);

        // Run the calculation function.
        for (int i = 0; i < m_buf_length; i++) {
            for (int j = 0; j < num_inputs; j++) {
                unit->mInBuf[j][i] = input_values[j];
            }
            unit->mOutBuf[0][i] = 0;
        }
        (unit->mCalcFunc)(unit, unit->mBufLength);

        // Copy the results into an output buffer.
        std::vector<float> result;
        result.resize(m_buf_length);
        for (int i = 0; i < m_buf_length; i++) {
            result[i] = unit->mOutBuf[0][i];
        }

        // Free up memory.
        for (int i = 0; i < num_ports; i++) {
            free(unit->mInput[i]);
        }
        for (int i = 0; i < num_ports; i++) {
            free(unit->mInBuf[i]);
        }
        free(unit->mInBuf);
        free(graph);
        free(unit);

        return result;
    }

    const double m_sample_rate;
    const int m_buf_length;
    World* m_world;

    static UnitDef* get_unit_def(const char* name) {
        int32 key[kSCNameByteLen];
        str4cpy(key, name);
        UnitDef* unit_def = gUnitDefLib->Get(key);
        if (unit_def == nullptr) {
            throw "UGen not found!";
        }
        return unit_def;
    }

};

BOOST_AUTO_TEST_CASE(nathan) {
    UGenRunner runner(48000.0, 64);

    runner.run_ugen("WhiteNoise", {});
    runner.run_ugen("PinkNoise", {});
    runner.run_ugen("BrownNoise", {});
    runner.run_ugen("Impulse", {0.0});
    runner.run_ugen("SinOsc", {440.0, 0.0});
}
