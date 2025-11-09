#pragma once

#include <map>

#include "PhoenixSim.h"
#include "Reflection.h"
#include "Worlds.h"

namespace Phoenix
{
    struct IDebugState;
    struct IDebugRenderer;
    class Session;
}

#define PHX_FEATURE_BEGIN(feature) \
    public: \
        using ThisType = feature; \
        static constexpr FName StaticName = #feature##_n; \
        virtual FName GetName() const override { return StaticName; } \
    private: \
        struct SFeatureDefinition { \
            static constexpr FName StaticName = #feature##_n; \
            static constexpr const char* StaticDisplayName = #feature; \
            static FeatureDefinition Construct() \
            { \
                Phoenix::FeatureDefinition definition; \
                definition.Name = StaticName; \
                definition.DisplayName = StaticDisplayName; \

#define PHX_FEATURE_END() \
                return definition; \
            } \
        }; \
        const FeatureDefinition& GetFeatureDefinition() override { static FeatureDefinition fd = SFeatureDefinition::Construct(); return fd; }

#define FEATURE_SESSION_BLOCK(block) definition.RegisterSessionBlock<block>();
#define FEATURE_WORLD_BLOCK(block) definition.RegisterWorldBlock<block>();
#define FEATURE_CHANNEL(...) definition.RegisterChannel(__VA_ARGS__);

namespace Phoenix
{
    struct Action;

    struct FeatureChannels
    {
#define PHX_DECLARE_CHANNEL(name) static constexpr FName name = #name##_n

        // Session
        PHX_DECLARE_CHANNEL(PreUpdate);
        PHX_DECLARE_CHANNEL(Update);
        PHX_DECLARE_CHANNEL(PostUpdate);
        PHX_DECLARE_CHANNEL(PreHandleAction);
        PHX_DECLARE_CHANNEL(HandleAction);
        PHX_DECLARE_CHANNEL(PostHandleAction);

        // World
        PHX_DECLARE_CHANNEL(WorldInitialize);
        PHX_DECLARE_CHANNEL(WorldShutdown);
        PHX_DECLARE_CHANNEL(PreWorldUpdate);
        PHX_DECLARE_CHANNEL(WorldUpdate);
        PHX_DECLARE_CHANNEL(PostWorldUpdate);
        PHX_DECLARE_CHANNEL(PreHandleWorldAction);
        PHX_DECLARE_CHANNEL(HandleWorldAction);
        PHX_DECLARE_CHANNEL(PostHandleWorldAction);

        PHX_DECLARE_CHANNEL(DebugRender);

#undef PHX_DECLARE_CHANNEL
    };

    struct PHOENIXSIM_API FeatureUpdateArgs
    {
        simtime_t SimTime = 0;
        dt_t StepHz = 0;
    };

    struct PHOENIXSIM_API FeatureActionArgs
    {
        simtime_t SimTime = 0;
        Action Action;
    };
    
    class PHOENIXSIM_API IFeature : public TSharedAsThis<IFeature>
    {
    public:

        virtual ~IFeature() {};

        // Gets the name of the feature.
        virtual FName GetName() const;

        virtual const struct FeatureDefinition& GetFeatureDefinition() = 0;

        // Gets the session that this feature belongs to.
        Session* GetSession() const;

        // Called when the feature is initialized.
        virtual void Initialize();

        // Called prior to the feature shutting down.
        virtual void Shutdown();

        // Called when a new world is created and gives the feature a chance to initialize.
        virtual void OnWorldInitialize(WorldRef world);

        // Called when a world is about to be released and gives the feature a chance to clean up.
        virtual void OnWorldShutdown(WorldRef world);

        // Called once per session step, before OnUpdate.
        virtual void OnPreUpdate(const FeatureUpdateArgs& args);

        // Called once per session step.
        virtual void OnUpdate(const FeatureUpdateArgs& args);

        // Called once per session step, after OnUpdate.
        virtual void OnPostUpdate(const FeatureUpdateArgs& args);

        // Called once per action sent to the session, before OnHandleAction.
        virtual bool OnPreHandleAction(const FeatureActionArgs& action);
 
        // Called once per action sent to the session.
        virtual bool OnHandleAction(const FeatureActionArgs& action);

        // Called once per action sent to the session, after OnHandleAction.
        virtual bool OnPostHandleAction(const FeatureActionArgs& action);

        // Called once per session step, per world, before OnWorldUpdate.
        virtual void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args);

        // Called once per session step, per world.
        virtual void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args);

        // Called once per session step, per world, after OnWorldUpdate.
        virtual void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args);

        // Called once per action sent to a specific world, before OnHandleWorldAction.
        virtual bool OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& action);

        // Called once per action sent to a specific world.
        virtual bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action);

        // Called once per action sent to a specific world, after OnHandleWorldAction.
        virtual bool OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& action);

        // Gives the feature the ability to render debug information for a given world.
        virtual void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer);

    protected:

        friend class Session;

        Session* Session = nullptr;
    };

    typedef IFeature* FeaturePtr;
    typedef const IFeature* FeatureConstPtr;
    typedef TSharedPtr<IFeature> FeatureSharedPtr;
    typedef TSharedPtr<const IFeature> FeatureSharedConstPtr;

    enum class EFeatureInsertPosition : uint8
    {
        Default,
        Before,
        After,
    };

    struct PHOENIXSIM_API FeatureInsertPosition
    {
        static const FeatureInsertPosition Default;
        FName FeatureName;
        EFeatureInsertPosition RelativePosition = EFeatureInsertPosition::Default;
    };

    struct PHOENIXSIM_API FeatureChannelInsertArgs
    {
        FeatureChannelInsertArgs() = default;

        FeatureChannelInsertArgs(
            const FName& channelName,
            const FeatureInsertPosition& position = {});

        FName Channel;
        FeatureInsertPosition InsertPosition;
    };

    struct PHOENIXSIM_API FeatureSetCtorArgs
    {
        TArray<FeatureSharedPtr> Features;
    };

    struct PHOENIXSIM_API FeatureDefinition : TypeDescriptor
    {
        BlockBuffer::CtorArgs SessionBlocks;
        BlockBuffer::CtorArgs WorldBlocks;
        TArray<FeatureChannelInsertArgs> Channels;
        TArray<FName> DependentFeatures;

        template <class TBlock>
        void RegisterSessionBlock()
        {
            SessionBlocks.RegisterBlock<TBlock>();
        }

        template <class TBlock>
        void RegisterWorldBlock()
        {
            WorldBlocks.RegisterBlock<TBlock>();
        }

        void RegisterChannel(FName channel, const FeatureInsertPosition& insertPosition = FeatureInsertPosition::Default)
        {
            Channels.emplace_back(channel, insertPosition);
        }
    };

    class PHOENIXSIM_API FeatureSet
    {
    public:

        FeatureSet(const FeatureSetCtorArgs& args);

        FeatureSharedPtr GetFeature(const FName& name) const;

        template <class TFeature>
        TSharedPtr<TFeature> GetFeature(const FName& name) const
        {
            return std::static_pointer_cast<TFeature>(GetFeature(name));
        }

        template <class TFeature>
        TSharedPtr<TFeature> GetFeature() const
        {
            return GetFeature<TFeature>(TFeature::StaticName);
        }

        TArray<FeatureSharedPtr> GetFeatures() const;

        // Gets an array containing all the names of the channels.
        TArray<FName> GetChannelNames() const;
        
        TArray<FeatureSharedPtr> GetChannel(const FName& channelName) const;
        const TArray<FeatureSharedPtr>& GetChannelRef(const FName& channelName) const;

        template <class TCallback>
        void ForEachFeatureInChannel(const FName& channelName, const TCallback& callback)
        {
            auto&& channelEntry = Channels.find(channelName);
            if (channelEntry == Channels.end())
                return;

            const auto& channel = channelEntry->second;
            for (auto&& feature : channel)
            {
                callback(*feature);
            }
        }

    private:
        
        void RegisterFeatureChannels(const TArray<FeatureSharedPtr>& featureDefs);

        static int32 FindChannelInsertIndex(
            const TArray<FeatureSharedPtr>& channelFeatures,
            const FeatureInsertPosition& insertPosition);

        TMap<FName, FeatureSharedPtr> Features;
        TMap<FName, TArray<FeatureSharedPtr>> Channels;
    };
}
