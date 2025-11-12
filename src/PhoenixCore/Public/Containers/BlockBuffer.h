
#pragma once

#include "Platform.h"
#include "Reflection.h"

namespace Phoenix
{
    enum class EBufferBlockType : uint8
    {
        Static,
        Dynamic,
        Scratch
    };

    class PHOENIXCORE_API BlockBuffer
    {
    public:

        struct BlockDefinition
        {
            FName Name;
            size_t Size = 0;
            uint8 Priority = 0;
            const TypeDescriptor* Type = nullptr;

            BlockDefinition(FName name, size_t size, uint8 priority, const TypeDescriptor* type)
                : Name(name), Size(size), Priority(priority), Type(type)
            {
            }
        };

        struct CtorArgs
        {
            template <class TBlock>
            void RegisterBlock()
            {
                const TypeDescriptor& type = TBlock::GetStaticTypeDescriptor();
                Definitions.emplace_back(type.GetName(), sizeof(TBlock), (uint8)TBlock::StaticBlockType, &type);
            }

            TArray<BlockDefinition> Definitions;
        };

        struct Block
        {
            Block(const BlockDefinition& definition);

            BlockDefinition Definition;
            size_t Offset = 0;
        };

        BlockBuffer();
        BlockBuffer(const CtorArgs& args);
        BlockBuffer(const BlockBuffer& other);
        BlockBuffer(BlockBuffer&& other) noexcept;

        BlockBuffer& operator=(const BlockBuffer& other);
        BlockBuffer& operator=(BlockBuffer&& other) noexcept;

        uint8* GetData();
        const uint8* GetData() const;

        size_t GetSize() const;

        const TArray<Block>& GetBlocks() const;

        const BlockDefinition* GetBlockDefinition(const FName& name) const;

        uint8* GetBlock(const FName& name);
        const uint8* GetBlock(const FName& name) const;

        template <class TBlock>
        TBlock* GetBlock(const FName& name)
        {
            return static_cast<TBlock>(GetBlock<TBlock>(name));
        }

        template <class TBlock>
        const TBlock* GetBlock(const FName& name) const
        {
            return static_cast<TBlock>(GetBlock<TBlock>(name));
        }

        template <class TBlock>
        TBlock* GetBlock()
        {
            return reinterpret_cast<TBlock*>(GetBlock(TBlock::StaticTypeName));
        }

        template <class TBlock>
        const TBlock* GetBlock() const
        {
            return reinterpret_cast<const TBlock*>(GetBlock(TBlock::StaticTypeName));
        }

        template <class TBlock>
        TBlock& GetBlockRef()
        {
            return *GetBlock<TBlock>();
        }

        template <class TBlock>
        const TBlock& GetBlockRef() const
        {
            return *GetBlock<TBlock>();
        }

    private:

        TArray<Block> Blocks;
        TUniquePtr<uint8[]> Data = nullptr;
        size_t Size = 0;
    };

    struct PHOENIXCORE_API BufferBlockBase
    {
        virtual ~BufferBlockBase() {}
        virtual const TypeDescriptor& GetTypeDescriptor() const = 0;
    };
}

#define PHX_DECLARE_BLOCK_BEGIN(block, type) \
    static constexpr EBufferBlockType StaticBlockType = type; \
    PHX_DECLARE_TYPE_BEGIN(block)

#define PHX_DECLARE_BLOCK_END() \
    PHX_DECLARE_TYPE_END()

#define PHX_DECLARE_BLOCK(block, type) \
    static constexpr FName StaticName = #block##_n; \
    static constexpr EBufferBlockType StaticType = type;

#define PHX_DECLARE_BLOCK_STATIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_BEGIN(block, EBufferBlockType::Static)

#define PHX_DECLARE_BLOCK_STATIC(block) \
    PHX_DECLARE_BLOCK_STATIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_END()

#define PHX_DECLARE_BLOCK_DYNAMIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_BEGIN(block, EBufferBlockType::Dynamic)

#define PHX_DECLARE_BLOCK_DYNAMIC(block) \
    PHX_DECLARE_BLOCK_DYNAMIC_BEGIN(block) \
    PHX_DECLARE_BLOCK_END()

#define PHX_DECLARE_BLOCK_SCRATCH_BEGIN(block) \
    PHX_DECLARE_BLOCK_BEGIN(block, EBufferBlockType::Scratch)

#define PHX_DECLARE_BLOCK_SCRATCH(block) \
    PHX_DECLARE_BLOCK_SCRATCH_BEGIN(block) \
    PHX_DECLARE_BLOCK_END()