
#pragma once

#include "PhoenixFeatures.h"
#include "Worlds.h"
#include "Containers/FixedArray.h"
#include "FixedPoint/FixedTransform.h"
#include "Utils.h"

#include <cstdint>  // For INT16_MAX

#ifndef ECS_MAX_ENTITIES
#define ECS_MAX_ENTITIES INT16_MAX
#endif

#ifndef ECS_MAX_TAGS
#define ECS_MAX_TAGS (INT16_MAX << 1)
#endif

#ifndef ECS_MAX_COMPONENTS
#define ECS_MAX_COMPONENTS (INT16_MAX << 1)
#endif

#ifndef ECS_MAX_COMPONENT_SIZE
#define ECS_MAX_COMPONENT_SIZE (64 - (sizeof(hash32_t) + sizeof(int32))) + 64
#endif

#define DECLARE_ECS_COMPONENT(component) \
    static constexpr FName StaticName = #component##_n;

#define DECLARE_ECS_SYSTEM(system) \
    static constexpr FName StaticName = #system##_n; \
    virtual FName GetName() const override { return StaticName; }

namespace Phoenix
{
    namespace ECS
    {
        struct FeatureECSDynamicBlock;
        typedef uint32 entityid_t;
        
        struct PHOENIXSIM_API EntityId
        {
            static const EntityId Invalid;

            constexpr EntityId() : Id(0) {}
            constexpr EntityId(entityid_t raw) : Id(raw) {}

            operator entityid_t() const;
            EntityId& operator=(const entityid_t& id);

        private:
            entityid_t Id;
        };

        struct PHOENIXSIM_API Entity
        {
            EntityId Id = EntityId::Invalid;
            FName Kind;
            int32 ComponentHead = INDEX_NONE;
            int32 TagHead = INDEX_NONE;
        };

        struct PHOENIXSIM_API EntityTag
        {
            FName TagName;
            int32 Next = INDEX_NONE;
        };

        struct PHOENIXSIM_API EntityComponent
        {
            // The name of the component type.
            FName TypeName;

            // The index of the next component owned by a given entity.
            int32 Next = INDEX_NONE;

            // The raw data of the component.
            uint8 Data[ECS_MAX_COMPONENT_SIZE];
        };

        struct PHOENIXSIM_API TransformComponent
        {
            DECLARE_ECS_COMPONENT(TransformComponent)

            // The id of another entity that the owning entity is attached to.
            // Note that this cannot be the entity that owns the body component.
            EntityId AttachParent = EntityId::Invalid;

            // The relative transform of the entity.
            // Relative to the origin if not attached to another entity.
            Transform2D Transform;

            // Morton z-code used for spacial sorting of entities.
            uint64 ZCode = 0;
        };

        template <class ...TComponents>
        class EntityComponentsContainer
        {
        public:
            using ElementType = std::tuple<Entity*, std::add_pointer_t<TComponents>...>;

            EntityComponentsContainer() = default;

            EntityComponentsContainer(WorldRef world)
            {
                Refresh(world);
            }

            void Refresh(WorldRef world);

            auto begin() { return Components.begin(); }
            auto end() { return Components.end(); }

        private:
            TFixedArray<ElementType, ECS_MAX_ENTITIES> Components;
        };

        struct PHOENIXSIM_API SystemUpdateArgs
        {
            simtime_t SimTime = 0;
            DeltaTime DeltaTime;
        };

        struct PHOENIXSIM_API SystemActionArgs
        {
            simtime_t SimTime = 0;
            Action Action;
        };

        class PHOENIXSIM_API ISystem
        {
        public:
            virtual ~ISystem() = default;

            virtual FName GetName() const { return FName::None; }

            virtual void OnPreUpdate(WorldRef world, const SystemUpdateArgs& args) {}
            virtual void OnUpdate(WorldRef world, const SystemUpdateArgs& args) {}
            virtual void OnPostUpdate(WorldRef world, const SystemUpdateArgs& args) {}

            virtual bool OnPreHandleAction(WorldRef world, const SystemActionArgs& args) { return false; }
            virtual bool OnHandleAction(WorldRef world, const SystemActionArgs& args) { return false; }
            virtual bool OnPostHandleAction(WorldRef world, const SystemActionArgs& args) { return false; }

            virtual void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) {}
        };

        struct PHOENIXSIM_API FeatureECSDynamicBlock : BufferBlockBase
        {
            PHX_DECLARE_BLOCK_DYNAMIC(FeatureECSDynamicBlock)

            TFixedArray<Entity, ECS_MAX_ENTITIES> Entities;
            TFixedArray<EntityTag, ECS_MAX_TAGS> Tags;
            TFixedArray<EntityComponent, ECS_MAX_COMPONENTS> Components;
            TFixedArray<EntityId, ECS_MAX_ENTITIES> Groups;
        };

        struct PHOENIXSIM_API EntityTransform
        {
            EntityId EntityId;
            TransformComponent* TransformComponent;
            uint64 ZCode = 0;
        };

        struct PHOENIXSIM_API FeatureECSScratchBlock : BufferBlockBase
        {
            PHX_DECLARE_BLOCK_SCRATCH(FeatureECSScratchBlock)

            EntityComponentsContainer<TransformComponent> EntityTransforms;
            TFixedArray<EntityTransform, ECS_MAX_ENTITIES> SortedEntities;
        };

        struct PHOENIXSIM_API FeatureECSCtorArgs
        {
            TArray<TSharedPtr<ISystem>> Systems;
        };

        class PHOENIXSIM_API FeatureECS : public IFeature
        {
            PHX_FEATURE_BEGIN(FeatureECS)
                FEATURE_WORLD_BLOCK(FeatureECSDynamicBlock)
                FEATURE_WORLD_BLOCK(FeatureECSScratchBlock)
                FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
                FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
                FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
                FEATURE_CHANNEL(FeatureChannels::PreHandleWorldAction)
                FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
                FEATURE_CHANNEL(FeatureChannels::PostHandleWorldAction)
                FEATURE_CHANNEL(FeatureChannels::DebugRender)
                PHX_REGISTER_FIELD(bool, bDebugDrawMortonCodeBoundaries)
                PHX_REGISTER_FIELD(bool, bDebugDrawEntityZCodes)
            PHX_FEATURE_END()

        public:

            FeatureECS();
            FeatureECS(const FeatureECSCtorArgs& args);
            
            void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
            void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
            void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

            bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

            void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) override;

            static bool IsEntityValid(WorldConstRef world, EntityId entityId);

            static int32 GetEntityIndex(EntityId entityId);
            
            static Entity* GetEntityPtr(WorldRef world, EntityId entityId);
            static const Entity* GetEntityPtr(WorldConstRef world, EntityId entityId);

            static Entity& GetEntityRef(WorldRef world, EntityId entityId);
            static const Entity& GetEntityRef(WorldConstRef world, EntityId entityId);

            static EntityId AcquireEntity(WorldRef world, FName kind);
            static bool ReleaseEntity(WorldRef world, EntityId entityId);

            static bool HasTag(WorldConstRef world, EntityId entityId, FName tagName);

            static bool AddTag(WorldRef world, EntityId entityId, FName tagName);
            static bool RemoveTag(WorldRef world, EntityId entityId, FName tagName);
            static uint32 RemoveAllTags(WorldRef world, EntityId entityId);

            template <class TCallback>
            static void ForEachTag(WorldConstRef world, EntityId entityId, const TCallback& callback)
            {
                const FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

                const Entity* entity = GetEntityPtr(world, entityId);
                if (!entity)
                {
                    return;
                }

                int32 tagIndex = entity->TagHead;
                while (tagIndex != INDEX_NONE)
                {
                    const EntityTag& tag = block.Tags[tagIndex];
                    if (!callback(tag, tagIndex))
                    {
                        return;
                    }
                    tagIndex = tag.Next;
                }
            }

            static int32 GetIndexOfComponent(WorldConstRef world, EntityId entityId, FName componentType);

            static EntityComponent* GetComponentPtr(WorldRef world, EntityId entityId, FName componentType);
            static const EntityComponent* GetComponentPtr(WorldConstRef world, EntityId entityId, FName componentType);

            static EntityComponent& GetComponentRef(WorldRef world, EntityId entityId, FName componentType);
            static const EntityComponent& GetComponentRef(WorldConstRef world, EntityId entityId, FName componentType);

            template <class T>
            static T* GetComponentDataPtr(WorldRef world, EntityId entityId)
            {
                EntityComponent* comp = GetComponentPtr(world, entityId, T::StaticName);
                if (!comp) return nullptr;
                return reinterpret_cast<T*>(&comp->Data[0]);
            }

            template <class T>
            static const T* GetComponentDataPtr(WorldConstRef world, EntityId entityId)
            {
                EntityComponent* comp = GetComponentPtr(world, entityId, T::StaticName);
                if (!comp) return nullptr;
                return reinterpret_cast<const T*>(&comp->Data[0]);
            }

            template <class T>
            static T* AddComponent(WorldRef world, EntityId entityId, const T& defaultValue = {})
            {
                static_assert(sizeof(T) < ECS_MAX_COMPONENT_SIZE);
                EntityComponent* comp = AddComponent(world, entityId, T::StaticName);
                if (!comp) return nullptr;
                T* typedData = reinterpret_cast<T*>(&comp->Data[0]);
                *typedData = defaultValue;
                return typedData;
            }

            template <class T, class ...TArgs>
            static T* EmplaceComponent(WorldRef world, EntityId entityId, const TArgs&...args)
            {
                static_assert(sizeof(T) < ECS_MAX_COMPONENT_SIZE);
                EntityComponent* comp = AddComponent(world, entityId, T::StaticName);
                if (!comp) return nullptr;
                T* typedData = reinterpret_cast<T*>(&comp->Data[0]);
                new (typedData) T(args...);
                return typedData;
            }

            static bool RemoveComponent(WorldRef world, EntityId entityId, FName componentType);

            static uint32 RemoveAllComponents(WorldRef world, EntityId entityId);

            template <class T>
            static void ForEachComponent(WorldRef world, EntityId entityId, const T& callback)
            {
                FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

                const Entity* entity = GetEntityPtr(world, entityId);
                if (!entity)
                {
                    return;
                }

                int32 compIndex = entity->ComponentHead;

                while (compIndex != INDEX_NONE)
                {
                    EntityComponent& comp = block.Components[compIndex];
                    if (!callback(comp, compIndex))
                    {
                        return;
                    }
                    compIndex = comp.Next;
                }
            }

            template <class T>
            static void ForEachComponent(WorldConstRef world, EntityId entityId, const T& callback)
            {
                const FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

                const Entity* entity = GetEntityPtr(world, entityId);
                if (!entity)
                {
                    return;
                }

                int32 compIndex = entity->ComponentHead;

                while (compIndex != INDEX_NONE)
                {
                    const EntityComponent& comp = block.Components[compIndex];
                    if (!callback(comp, compIndex))
                    {
                        return;
                    }
                    compIndex = comp.Next;
                }
            }

            void RegisterSystem(const TSharedPtr<ISystem>& system);

            static void GetWorldTransform(WorldConstRef world, EntityId entityId, Transform2D& outTransform);

            static void QueryEntitiesInRange(WorldConstRef& world, const Vec2& pos, Distance range, TArray<EntityTransform>& outEntities);

            bool bDebugDrawMortonCodeBoundaries = false;
            bool bDebugDrawEntityZCodes = false;

        private:

            static EntityComponent* AddComponent(WorldRef world, EntityId entityId, FName componentType);

            static void SortEntitiesByZCode(WorldRef world);

            static void CompactWorldBuffer(WorldRef world);

            TArray<TSharedPtr<ISystem>> Systems;
        };

        template <class ... TComponents>
        void EntityComponentsContainer<TComponents...>::Refresh(WorldRef world)
        {
            FeatureECSDynamicBlock& block = world.GetBlockRef<FeatureECSDynamicBlock>();

            Components.Reset();

            for (Entity& entity : block.Entities)
            {
                if (entity.Id == EntityId::Invalid)
                    continue;
            
                auto components = std::make_tuple(&entity, FeatureECS::GetComponentDataPtr<TComponents>(world, entity.Id)...);
                if (ContainsNullptr(components))
                    continue;

                Components.PushBack(components);
            }
        }
    }
}