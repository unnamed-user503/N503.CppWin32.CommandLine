#pragma once
#include <cstdint>
#include <string_view>

namespace N503::CommandLine::Lexing
{
    struct Token
    {
        enum class Bits : std::uint32_t
        {
            Null = 0,

            // Categories (下位 16-bit)
            Literal = (1 << 0),
            Delimiter = (1 << 1),
            DoubleQuotation = (1 << 2),
            Backslash = (1 << 3),

            // Details (上位 16-bit)
            EscapeSequence = ((1 << 0) << 16) | Literal,
            ApplicationNameDelimiter = ((1 << 1) << 16) | Delimiter,
            DoubleQuotationStart = ((1 << 2) << 16) | DoubleQuotation,
            DoubleQuotationClose = ((1 << 3) << 16) | DoubleQuotation,
            BackslashStart = ((1 << 4) << 16) | Backslash,
            BackslashClose = ((1 << 5) << 16) | Backslash,
            BackslashCloseForEven = ((1 << 6) << 16) | Backslash,
            BackslashCloseForOdd = ((1 << 7) << 16) | Backslash,
            UnterminatedQuote = ((1 << 8) << 16) | DoubleQuotation,
        };

        Bits code = Bits::Null;
        std::string_view view;

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return code != Bits::Null;
        }
    };
} // namespace N503::CommandLine::Lexing
