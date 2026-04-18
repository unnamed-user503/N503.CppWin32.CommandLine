#pragma once
#include <cstdint>
#include <variant>

namespace N503::CommandLine::Lexing::State
{
    struct Initial
    {
        bool IsProcessingQuote{ false };
    };
    struct Normal
    {
    };
    struct Quoted
    {
    };
    struct Backslash
    {
        std::uint32_t Count{ 1 };
    };

    using Any = std::variant<Initial, Normal, Quoted, Backslash>;
} // namespace N503::CommandLine::Lexing::State
