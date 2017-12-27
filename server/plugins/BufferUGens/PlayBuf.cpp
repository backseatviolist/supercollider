/*
    SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
    http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "BufferUGens.hpp"
#include "PlayBuf.hpp"

void PlayBuf_Ctor(PlayBuf *unit)
{
    if (INRATE(1) == calc_FullRate) {
        if (INRATE(2) == calc_FullRate) {
            SETCALC(PlayBuf_next_aa);
        } else {
            SETCALC(PlayBuf_next_ak);
        }
    } else {
        if (INRATE(2) == calc_FullRate) {
            SETCALC(PlayBuf_next_ka);
        } else {
            SETCALC(PlayBuf_next_kk);
        }
    }

    unit->m_fbufnum = -1e9f;
    unit->m_failedBufNum = -1e9f;
    unit->m_prevtrig = 0.;
    unit->m_phase = ZIN0(3);

    ClearUnitOutputs(unit, 1);
}


void PlayBuf_next_aa(PlayBuf *unit, int inNumSamples)
{
    float *ratein  = ZIN(1);
    float *trigin  = ZIN(2);
    int32 loop     = (int32)ZIN0(4);

    float fbufnum  = ZIN0(0);
    if (fbufnum != unit->m_fbufnum) {
        uint32 bufnum = (int)fbufnum;
        World *world = unit->mWorld;
        if (bufnum >= world->mNumSndBufs) bufnum = 0;
        unit->m_fbufnum = fbufnum;
        unit->m_buf = world->mSndBufs + bufnum;
    }
    const SndBuf *buf = unit->m_buf;
    ACQUIRE_SNDBUF_SHARED(buf);
    const float *bufData __attribute__((__unused__)) = buf->data;
    uint32 bufChannels __attribute__((__unused__)) = buf->channels;
    uint32 bufSamples __attribute__((__unused__)) = buf->samples;
    uint32 bufFrames = buf->frames;
    int mask __attribute__((__unused__)) = buf->mask;
    int guardFrame __attribute__((__unused__)) = bufFrames - 2;

    int numOutputs = unit->mNumOutputs;

    CHECK_BUFFER_DATA;

    double loopMax = (double)(loop ? bufFrames : bufFrames - 1);
    double phase = unit->m_phase;
    float prevtrig = unit->m_prevtrig;

    for (int i=0; i<inNumSamples; ++i) {
        float trig = ZXP(trigin);
        if (trig > 0.f && prevtrig <= 0.f) {
            unit->mDone = false;
            phase = ZIN0(3);
        }
        prevtrig = trig;

        LOOP_BODY_4(i)

        phase += ZXP(ratein);
    }
    RELEASE_SNDBUF_SHARED(buf);

    if(unit->mDone)
        DoneAction((int)ZIN0(5), unit);
    unit->m_phase = phase;
    unit->m_prevtrig = prevtrig;
}

void PlayBuf_next_ak(PlayBuf *unit, int inNumSamples)
{
    float *ratein  = ZIN(1);
    float trig     = ZIN0(2);
    int32 loop     = (int32)ZIN0(4);

    float fbufnum  = ZIN0(0);
    if (fbufnum != unit->m_fbufnum) {
        uint32 bufnum = (int)fbufnum;
        World *world = unit->mWorld;
        if (bufnum >= world->mNumSndBufs) bufnum = 0;
        unit->m_fbufnum = fbufnum;
        unit->m_buf = world->mSndBufs + bufnum;
    }
    const SndBuf *buf = unit->m_buf;
    ACQUIRE_SNDBUF_SHARED(buf);
    const float *bufData __attribute__((__unused__)) = buf->data;
    uint32 bufChannels __attribute__((__unused__)) = buf->channels;
    uint32 bufSamples __attribute__((__unused__)) = buf->samples;
    uint32 bufFrames = buf->frames;
    int mask __attribute__((__unused__)) = buf->mask;
    int guardFrame __attribute__((__unused__)) = bufFrames - 2;

    int numOutputs = unit->mNumOutputs;

    CHECK_BUFFER_DATA

    double loopMax = (double)(loop ? bufFrames : bufFrames - 1);
    double phase = unit->m_phase;
    if(phase == -1.) phase = bufFrames;
    if (trig > 0.f && unit->m_prevtrig <= 0.f) {
        unit->mDone = false;
        phase = ZIN0(3);
    }
    unit->m_prevtrig = trig;
    for (int i=0; i<inNumSamples; ++i) {

        LOOP_BODY_4(i)

        phase += ZXP(ratein);
    }
    RELEASE_SNDBUF_SHARED(buf);
    if(unit->mDone)
        DoneAction((int)ZIN0(5), unit);
    unit->m_phase = phase;
}

void PlayBuf_next_kk(PlayBuf *unit, int inNumSamples)
{
    float rate     = ZIN0(1);
    float trig     = ZIN0(2);
    int32 loop     = (int32)ZIN0(4);

    GET_BUF_SHARED
    int numOutputs = unit->mNumOutputs;

    CHECK_BUFFER_DATA

    double loopMax = (double)(loop ? bufFrames : bufFrames - 1);
    double phase = unit->m_phase;
    if (trig > 0.f && unit->m_prevtrig <= 0.f) {
        unit->mDone = false;
        phase = ZIN0(3);
    }
    unit->m_prevtrig = trig;
    for (int i=0; i<inNumSamples; ++i) {
        LOOP_BODY_4(i)

        phase += rate;
    }
    if(unit->mDone)
        DoneAction((int)ZIN0(5), unit);
    unit->m_phase = phase;
}

void PlayBuf_next_ka(PlayBuf *unit, int inNumSamples)
{
    float rate     = ZIN0(1);
    float *trigin  = ZIN(2);
    int32 loop     = (int32)ZIN0(4);

    GET_BUF_SHARED
    int numOutputs = unit->mNumOutputs;

    CHECK_BUFFER_DATA

    double loopMax = (double)(loop ? bufFrames : bufFrames - 1);
    double phase = unit->m_phase;
    float prevtrig = unit->m_prevtrig;
    for (int i=0; i<inNumSamples; ++i) {
        float trig = ZXP(trigin);
        if (trig > 0.f && prevtrig <= 0.f) {
            unit->mDone = false;
            if (INRATE(3) == calc_FullRate) phase = IN(3)[i];
            else phase = ZIN0(3);
        }
        prevtrig = trig;

        LOOP_BODY_4(i)

        phase += rate;
    }
    if(unit->mDone)
        DoneAction((int)ZIN0(5), unit);
    unit->m_phase = phase;
    unit->m_prevtrig = prevtrig;
}
