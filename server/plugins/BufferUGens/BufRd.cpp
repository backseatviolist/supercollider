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
#include "BufRd.hpp"

void BufRd_Ctor(BufRd *unit)
{
    int interp = (int)ZIN0(3);
    switch (interp) {
        case 1 : SETCALC(BufRd_next_1); break;
        case 2 : SETCALC(BufRd_next_2); break;
        default : SETCALC(BufRd_next_4); break;
    }

    unit->m_fbufnum = -1e9f;
    unit->m_failedBufNum = -1e9f;

    BufRd_next_1(unit, 1);
}

void BufRd_next_4(BufRd *unit, int inNumSamples)
{
    float *phasein = ZIN(1);
    int32 loop     = (int32)ZIN0(2);

    GET_BUF_SHARED
    uint32 numOutputs = unit->mNumOutputs;

    CHECK_BUFFER_DATA

    double loopMax = (double)(loop ? bufFrames : bufFrames - 1);

    for (int i=0; i<inNumSamples; ++i) {
        double phase = ZXP(phasein);
        LOOP_BODY_4(i)
    }
}

void BufRd_next_2(BufRd *unit, int inNumSamples)
{
    float *phasein = ZIN(1);
    int32 loop     = (int32)ZIN0(2);

    GET_BUF_SHARED
    uint32 numOutputs = unit->mNumOutputs;

    CHECK_BUFFER_DATA

    double loopMax = (double)(loop ? bufFrames : bufFrames - 1);

    for (int i=0; i<inNumSamples; ++i) {
        double phase = ZXP(phasein);
        LOOP_BODY_2(i)
    }
}

void BufRd_next_1(BufRd *unit, int inNumSamples)
{
    float *phasein = ZIN(1);
    int32 loop     = (int32)ZIN0(2);

    GET_BUF_SHARED
    uint32 numOutputs = unit->mNumOutputs;

    CHECK_BUFFER_DATA

    double loopMax = (double)(loop ? bufFrames : bufFrames - 1);

    for (int i=0; i<inNumSamples; ++i) {
        double phase = ZXP(phasein);
        LOOP_BODY_1(i)
    }
}
