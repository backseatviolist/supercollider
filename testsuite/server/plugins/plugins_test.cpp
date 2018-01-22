#include <iostream>
#include <boost/test/unit_test.hpp>
#include "SC_WorldOptions.h"
#include "SC_World.h"
#include "SC_Prototypes.h"
#include "SC_Errors.h"
#include "SC_Node.h"

BOOST_AUTO_TEST_CASE(nathan) {
    double sample_rate = 44100.0;
    int buf_length = 64;

    WorldOptions options = kDefaultWorldOptions;
    options.mBufLength = buf_length;
    options.mRealTime = false;
    options.mNumInputBusChannels = 1;
    options.mNumOutputBusChannels = 1;
    World* world = World_New(&options);
    World_SetSampleRate(world, sample_rate);
    World_Start(world);

    float* input_bus = world->mAudioBus + world->mNumOutputs * buf_length;
    float* output_bus = world->mAudioBus;

    Node* root_node = World_GetNode(world, 0);
    if (!root_node->mIsGroup) {
        std::cout << "what???" << std::endl;
        return;
    }
    Group* root_group = (Group*)root_node;



/*
    Graph_New(world, graph_def, node_id, &args, &graph, arg_type);
    Group_AddHead(root_node, &graph->mNode);
*/

    // World_Run(world);

    // for (int i = 0; i < buf_length; i++) {
    //     std::cout << outputBuses[i] << std::endl;
    // }

    World_Cleanup(world, true);
}
