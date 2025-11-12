
#pragma once

#include "Platform.h"

namespace Phoenix
{
    template <size_t N, size_t ChunkMaxSize>
    struct TFixedChunkAllocator
    {
        static constexpr auto Capacity = N;

        struct Handle
        {
            uint32 Id = -1;

            constexpr Handle() = default;
            constexpr Handle(uint32 id) : Id(id) {}
        };

        struct Chunk
        {
            uint32 Id = 0;
            uint32 UserData = 0;
            uint8 Data[ChunkMaxSize] = {};
        };

        constexpr uint32 Num() const
        {
            return NumChunks;
        }

        constexpr bool IsEmpty() const
        {
            return NumChunks == 0;
        }

        constexpr bool IsFull() const
        {
            return NumChunks == Capacity;
        }

        constexpr bool IsValid(Handle handle) const
        {
            return handle.Id <= Capacity && Chunks[handle.Id].Id == handle.Id;
        }

        Handle Allocate(uint32 userData)
        {
            if (IsFull())
            {
                return Handle();
            }

            Chunk& chunk = Chunks[NumChunks];
            memset(chunk.Data, 0, ChunkMaxSize);
            chunk.Id = NumChunks;
            chunk.UserData = userData;
            ++NumChunks; 

            return { chunk.Id };
        }

        template <class T, class ...TArgs>
        Handle Allocate(uint32 userData, TArgs&& ...args)
        {
            constexpr auto asdf = sizeof(T);
            constexpr auto asdf2 = ChunkMaxSize;
            PHX_ASSERT(sizeof(T) <= ChunkMaxSize);

            if (IsFull())
            {
                return Handle();
            }

            Chunk& chunk = Chunks[NumChunks]; 
            new (chunk.Data) T(std::forward<TArgs>(args)...);
            chunk.Id = NumChunks;
            chunk.UserData = userData;
            ++NumChunks; 

            return { chunk.Id };
        }

        void* GetPtr(Handle handle)
        {
            if (!IsValid(handle))
            {
                return nullptr;
            }

            return Chunks[handle.Id].Data;
        }

        const void* GetPtr(Handle handle) const
        {
            if (!IsValid(handle))
            {
                return nullptr;
            }

            return Chunks[handle.Id].Data;
        }

        template <class T>
        T* GetPtr(Handle handle)
        {
            return static_cast<T*>(GetPtr(handle));
        }

        template <class T>
        const T* GetPtr(Handle handle) const
        {
            return static_cast<const T*>(GetPtr(handle));
        }

        Chunk Chunks[Capacity] = {};
        uint32 NumChunks = 0;
    };
}
