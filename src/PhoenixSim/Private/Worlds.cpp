
#include "Worlds.h"

#include <algorithm>
#include <memory>

#include "PhoenixFeatures.h"
#include "Profiling.h"


using namespace Phoenix;

World::World(const WorldCtorArgs& args)
    : Name(args.Name)
    , Buffer(args.Blocks)
{
}

World::World(const World& other)
    : Name(other.Name)
    , Buffer(other.Buffer)
{
}

World::World(World&& other) noexcept
    : Name(other.Name)
    , Buffer(std::move(other.Buffer))
{
}

FName World::GetName() const
{
    return Name;
}

World& World::operator=(const World& other)
{
    Buffer = other.Buffer;
    return *this;
}

World& World::operator=(World&& other) noexcept
{
    Buffer = std::move(other.Buffer);
    return *this;
}

BlockBuffer& World::GetBuffer()
{
    return Buffer;
}

const BlockBuffer& World::GetBuffer() const
{
    return Buffer;
}

uint8_t* World::GetBlock(const FName& name)
{
    return Buffer.GetBlock(name);
}

const uint8_t* World::GetBlock(const FName& name) const
{
    return Buffer.GetBlock(name);
}

WorldManager::WorldManager(const WorldManagerCtorArgs& args)
    : FeatureSet(args.FeatureSet)
    , OnPostWorldUpdate(args.OnPostWorldUpdate)
{
    WorldBufferBlockArgs.RegisterBlock<WorldDynamicBlock>();

    for (const FeatureSharedPtr& feature : FeatureSet->GetFeatures())
    {
        FeatureDefinition worldFeatureDef = feature->GetFeatureDefinition();
        for (const BlockBuffer::BlockDefinition& blockArgs : worldFeatureDef.WorldBlocks.Definitions)
        {
            WorldBufferBlockArgs.Definitions.push_back(blockArgs);
        }
    }
}

WorldManager::~WorldManager()
{
}

WorldSharedPtr WorldManager::NewWorld(const FName& name)
{
    WorldSharedPtr world = GetWorld(name);
    if (world)
    {
        // TODO (jfarris): world with this name already exists!
        return world;
    }

    WorldCtorArgs worldCtorArgs;
    worldCtorArgs.Name = name;
    worldCtorArgs.Blocks = WorldBufferBlockArgs;

    world = std::make_shared<World>(worldCtorArgs);
    Worlds.push_back(world);

    InitializeWorld(*world);

    return world;
}

WorldSharedPtr WorldManager::GetWorld(const FName& name) const
{
    for (const WorldSharedPtr& world : Worlds)
    {
        if (world->GetName() == name)
            return world;
    }
    return nullptr;
}

WorldSharedPtr WorldManager::GetPrimaryWorld() const
{
    return Worlds[0];
}

void WorldManager::Step(const WorldStepArgs& args)
{
    TArray<WorldSharedPtr> worlds;

    if (args.WorldName != FName::None)
    {
        if (WorldSharedPtr world = GetWorld(args.WorldName))
        {
            worlds.push_back(world);
        }
    }
    else
    {
        worlds = Worlds;
    }

    // TODO (jfarris): parallelize
    for (const WorldSharedPtr& world : worlds)
    {
        UpdateWorld(*world, args.SimTime, args.StepHz);
    }
}

void WorldManager::SendAction(const WorldSendActionArgs& args)
{
    TArray<WorldSharedPtr> worlds;

    if (args.WorldName != FName::None)
    {
        if (WorldSharedPtr world = GetWorld(args.WorldName))
        {
            worlds.push_back(world);
        }
    }
    else
    {
        worlds = Worlds;
    }

    // TODO (jfarris): parallelize
    for (const WorldSharedPtr& world : worlds)
    {
        SendActionToWorld(*world, args.Action);
    }
}

void WorldManager::InitializeWorld(WorldRef world) const
{
    TArray<FeatureSharedPtr> channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::WorldInitialize);
    for (const FeatureSharedPtr& feature : channelFeatures)
    {
        feature->OnWorldInitialize(world);
    }
}

void WorldManager::ShutdownWorld(WorldRef world) const
{
    TArray<FeatureSharedPtr> channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::WorldShutdown);
    for (const FeatureSharedPtr& feature : channelFeatures)
    {
        feature->OnWorldShutdown(world);
    }
}

void WorldManager::UpdateWorld(WorldRef world, simtime_t time, clock_t stepHz) const
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureUpdateArgs updateArgs;
    updateArgs.SimTime = time;
    updateArgs.StepHz = stepHz;
    
    // Pre-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreWorldUpdate");

        const TArray<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PreWorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreWorldUpdate(world, updateArgs);
        }
    }

    // Update
    {
        PHX_PROFILE_ZONE_SCOPED_N("WorldUpdate");

        const TArray<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::WorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnWorldUpdate(world, updateArgs);
        }
    }

    // Post-update
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostWorldUpdate");

        const TArray<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PostWorldUpdate);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPostWorldUpdate(world, updateArgs);
        }
    }

    OnPostWorldUpdate(world);
}

void WorldManager::SendActionToWorld(WorldRef world, const Action& action) const
{
    PHX_PROFILE_ZONE_SCOPED;

    FeatureActionArgs actionArgs;
    actionArgs.Action = action;

    // Pre handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("PreHandleWorldAction");

        const TArray<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PreHandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPreHandleWorldAction(world, actionArgs);
        }
    }

    // Handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("HandleWorldAction");

        const TArray<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::HandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnHandleWorldAction(world, actionArgs);
        }
    }

    // Post handle action
    {
        PHX_PROFILE_ZONE_SCOPED_N("PostHandleWorldAction");

        const TArray<FeatureSharedPtr>& channelFeatures = FeatureSet->GetChannelRef(FeatureChannels::PostHandleWorldAction);
        for (const FeatureSharedPtr& feature : channelFeatures)
        {
            feature->OnPostHandleWorldAction(world, actionArgs);
        }
    }
}
