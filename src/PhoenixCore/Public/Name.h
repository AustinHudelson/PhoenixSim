
#pragma once

#include <stdlib.h>

#include "Platform.h"
#include "Hashing.h"

namespace Phoenix
{
    struct PHOENIXCORE_API FName
    {
        static const FName None;
        static const FName Empty;

        constexpr FName() = default;
        constexpr FName(hash32_t hash) : Value(hash) {}

        constexpr FName(const char* chars, size_t len)
            : Value(Hashing::FNV1A32(chars, len))
        {
#if DEBUG
            for (size_t i = 0; i < len && i < _countof(Debug); ++i)
                Debug[i] = *(chars + i);
#endif
        }

        explicit operator hash32_t() const;
        bool operator==(const FName& other) const;
        bool operator!=(const FName& other) const;
        std::strong_ordering operator<=>(const FName& other) const;

        FName operator+(const FName& other) const;
        FName& operator+=(const FName& other);

    private:
        hash32_t Value = 0;

#if DEBUG
    public:
        char Debug[64] = {};
#endif
    };

    PHOENIXCORE_API constexpr FName operator ""_n(const char* chars, size_t len)
    {
        return FName(chars, len);
    }
}


template <>
struct std::hash<Phoenix::FName>
{
    std::size_t operator()(const Phoenix::FName& value) const noexcept
    {
        return std::hash<Phoenix::hash32_t>{}((Phoenix::hash32_t)value);
    }
};
