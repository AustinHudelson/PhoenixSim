
#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "DLLExport.h"
#include "PhoenixFeatures.h"
#include "LuaFP64.h"

namespace Phoenix
{
    struct PHOENIXLUA_API FeatureLuaDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_DYNAMIC(FeatureLuaDynamicBlock)

        sol::state State;
    };

    class PHOENIXLUA_API FeatureLua : public IFeature
    {
    public:

        PHX_FEATURE_BEGIN(FeatureLua)
            FEATURE_SESSION_BLOCK(FeatureLuaDynamicBlock)
            FEATURE_CHANNEL(FeatureChannels::PreUpdate)
            FEATURE_CHANNEL(FeatureChannels::Update)
            FEATURE_CHANNEL(FeatureChannels::PostUpdate)
            FEATURE_CHANNEL(FeatureChannels::PreHandleAction)
            FEATURE_CHANNEL(FeatureChannels::HandleAction)
            FEATURE_CHANNEL(FeatureChannels::PostHandleAction)
            FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
            FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
            FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
            FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
            FEATURE_CHANNEL(FeatureChannels::PreHandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::PostHandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::DebugRender)
        PHX_FEATURE_END()

        void Initialize() override;
        void Shutdown() override;

        void OnPreUpdate(const FeatureUpdateArgs& args) override;
        void OnUpdate(const FeatureUpdateArgs& args) override;
        void OnPostUpdate(const FeatureUpdateArgs& args) override;

        bool OnPreHandleAction(const FeatureActionArgs& action) override;
        bool OnHandleAction(const FeatureActionArgs& action) override;
        bool OnPostHandleAction(const FeatureActionArgs& action) override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        bool OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
        bool OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;
    };
}
