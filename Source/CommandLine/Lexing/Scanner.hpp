#pragma once
#include "../Character/Reader.hpp"
#include "State.hpp"
#include "Token.hpp"
#include <variant>
#include <vector>

namespace N503::CommandLine::Lexing
{
    class Scanner final
    {
    public:
        explicit Scanner(Character::Reader& reader) : m_Reader(reader)
        {
            m_Stack.emplace_back(State::Initial{});
        }

        Token Next()
        {
            if (!m_Reader.CanRead() || m_Stack.empty())
            {
                return {};
            }
            return std::visit([this](auto& s)
            {
                return Dispatch(s);
            }, m_Stack.back());
        }

    private:
        Character::Reader& m_Reader;
        std::vector<State::Any> m_Stack;

        // 1. Initial (ApplicationName) 状態
        Token Dispatch(State::Initial& s)
        {
            const auto start = m_Reader.Tell();
            char current = m_Reader.Read();

            if (current == '"')
            {
                s.IsProcessingQuote = !s.IsProcessingQuote;
                return { Token::Bits::DoubleQuotation, m_Reader.ViewSince(start) };
            }

            if ((current == ' ' || current == '\t') && !s.IsProcessingQuote)
            {
                m_Stack.back() = State::Normal{}; // 最初のデリミタで通常状態へ
                return { Token::Bits::ApplicationNameDelimiter, m_Reader.ViewSince(start) };
            }
            return { Token::Bits::Literal, m_Reader.ViewSince(start) };
        }

        // 2. Normal 状態
        Token Dispatch(State::Normal&)
        {
            const auto start = m_Reader.Tell();
            char current = m_Reader.Read();

            if (current == '"')
            {
                m_Stack.emplace_back(State::Quoted{});
                return { Token::Bits::DoubleQuotationStart, m_Reader.ViewSince(start) };
            }
            if (current == ' ' || current == '\t')
            {
                return { Token::Bits::Delimiter, m_Reader.ViewSince(start) };
            }
            if (current == '\\')
            {
                if (m_Reader.Peek() == '"')
                {
                    m_Reader.Advance(); // \"
                    return { Token::Bits::EscapeSequence, m_Reader.ViewSince(start) };
                }
                m_Stack.emplace_back(State::Backslash{});
                return { Token::Bits::BackslashStart, m_Reader.ViewSince(start) };
            }
            return { Token::Bits::Literal, m_Reader.ViewSince(start) };
        }

        // 3. Quoted 状態
        Token Dispatch(State::Quoted&)
        {
            const auto start = m_Reader.Tell();
            if (!m_Reader.CanRead())
            {
                m_Stack.pop_back();
                return { Token::Bits::UnterminatedQuote, {} };
            }

            char current = m_Reader.Read();
            if (current == '"')
            {
                if (m_Reader.Peek() == '"')
                {
                    m_Reader.Advance(); // "" (MSVC特有のエスケープ)
                    return { Token::Bits::EscapeSequence, m_Reader.ViewSince(start) };
                }
                m_Stack.pop_back();
                return { Token::Bits::DoubleQuotationClose, m_Reader.ViewSince(start) };
            }
            if (current == '\\')
            {
                if (m_Reader.Peek() == '"')
                {
                    m_Reader.Advance();
                    return { Token::Bits::EscapeSequence, m_Reader.ViewSince(start) };
                }
                m_Stack.emplace_back(State::Backslash{});
                return { Token::Bits::BackslashStart, m_Reader.ViewSince(start) };
            }
            return { Token::Bits::Literal, m_Reader.ViewSince(start) };
        }

        // 4. Backslash 状態 (MSVC規則)
        Token Dispatch(State::Backslash& s)
        {
            const auto start = m_Reader.Tell();
            if (m_Reader.Peek() == '\\')
            {
                m_Reader.Advance();
                s.Count++;
                return { Token::Bits::Backslash, m_Reader.ViewSince(start) };
            }

            if (m_Reader.Peek() == '"')
            {
                auto code = (s.Count % 2 == 0) ? Token::Bits::BackslashCloseForEven : Token::Bits::BackslashCloseForOdd;
                if (code == Token::Bits::BackslashCloseForOdd)
                {
                    m_Reader.Advance();
                }

                m_Stack.pop_back();
                return { code, m_Reader.ViewSince(start) };
            }

            m_Stack.pop_back();
            return { Token::Bits::BackslashClose, m_Reader.ViewSince(start) };
        }
    };
} // namespace N503::CommandLine::Lexing
