
#pragma once

#include <algorithm>
#include <cstring>
#include <ctime>
#include <limits>

#include "Platform.h"

namespace Phoenix
{
    struct FPSCalc
    {
        static constexpr uint32 NumFrames = 60;

        uint64 TotalTicks = 0;
        clock_t Time = 0;

        double SecPerFrameAccum = 0.0;
        double SecPerFrame[NumFrames] = { 0.0 };
        uint32 SecPerFrameIdx = 0;
        uint32 SecPerFrameCount = 0;

        double Framerate = 0.0;

        FPSCalc()
        {
            Reset();
        }

        void Reset()
        {
            TotalTicks = 0;
            Time = PHX_CLOCK();
            SecPerFrameAccum = 0.0;
            memset(&SecPerFrame[0], 0, NumFrames * sizeof(double));
            SecPerFrameIdx = 0;
            SecPerFrameCount = 0;
            Framerate = 0.0;
        }

        double Tick()
        {
            ++TotalTicks;

            clock_t currTime = PHX_CLOCK();
            clock_t deltaTime = currTime - Time;
            Time = currTime;

            double dt = (double)deltaTime / (double)CLOCKS_PER_SEC;

            SecPerFrameAccum += dt - SecPerFrame[SecPerFrameIdx];
            SecPerFrame[SecPerFrameIdx] = dt;
            SecPerFrameIdx = (SecPerFrameIdx + 1) % NumFrames;
            SecPerFrameCount = std::min(SecPerFrameCount + 1, NumFrames);
            Framerate = (SecPerFrameAccum > 0.0) ? (1.0 / (SecPerFrameAccum / (double)SecPerFrameCount)) : std::numeric_limits<double>::max();

            return Framerate;
        }
    };
}
