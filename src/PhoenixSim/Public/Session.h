#pragma once

#include <shared_mutex>

#include "PhoenixFeatures.h"
#include "FPSCalc.h"

namespace Phoenix
{
    class WorldManager;
}

namespace Phoenix
{
    struct PHOENIXSIM_API SessionCtorArgs
    {
        FeatureSetCtorArgs FeatureSetArgs;
        PostWorldUpdateDelegate OnPostWorldUpdate;
    };

    struct PHOENIXSIM_API SessionStepArgs
    {
        clock_t DeltaTime = 0;
        clock_t StepHz = 1000 / 60;

        // Optionally only step this world.
        FName WorldName = FName::None;
    };

    class PHOENIXSIM_API Session
    {
    public:

        Session(const SessionCtorArgs& args);
        ~Session();

        void Initialize();
        void Shutdown();

        void QueueAction(const Action& action);

        void Tick(const SessionStepArgs& args);
        void Step(const SessionStepArgs& args);

        BlockBuffer* GetBuffer();
        const BlockBuffer* GetBuffer() const;

        clock_t GetCurrTime() const;
        clock_t GetStartTime() const;
        clock_t GetLastStepTime() const;
        simtime_t GetSimTime() const;
        double GetFramerate() const;

        FeatureSet* GetFeatureSet() const;
        WorldManager* GetWorldManager() const;

    private:

        void ProcessActions(simtime_t time);

        void UpdateSession(simtime_t time, clock_t stepHz) const;

        TSharedPtr<FeatureSet> FeatureSet;
        TSharedPtr<WorldManager> WorldManager;

        TArray<TTuple<simtime_t, Action>> ActionQueue;
        std::shared_mutex ActionQueueMutex;

        clock_t StartTime = 0;
        clock_t CurrTickTime = 0;
        int64 AccTickTime = 0;
        clock_t LastStepTime = 0;
        simtime_t SimTime = 0;

        // Steps per second
        FPSCalc FPSCalc;

        TUniquePtr<BlockBuffer> SessionBuffer;
    };
}

