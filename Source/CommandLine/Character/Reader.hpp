#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace N503::CommandLine::Character
{

    /// @brief 文字列ソースから 1 文字ずつ、あるいは条件付きで読み取るクラス。
    class Reader final
    {
    public:
        explicit constexpr Reader(std::string_view source) noexcept : m_Source(source), m_Offset(0)
        {
        }

        /// @brief 読み取り可能な文字が残っているか確認します。
        /// @return
        [[nodiscard]]
        constexpr bool CanRead() const noexcept
        {
            return m_Offset < m_Source.size();
        }

        /// @brief 現在の文字を読み取り、オフセットを進めます。
        /// @return
        constexpr char Read() noexcept
        {
            return CanRead() ? m_Source[m_Offset++] : '\0';
        }

        /// @brief 述語が真である間、文字を読み飛ばし、その範囲を返します。
        /// @tparam F
        /// @param predicate
        /// @return
        template <typename F> constexpr std::string_view ReadWhile(F&& predicate) noexcept
        {
            const auto start = m_Offset;
            while (CanRead() && predicate(Peek()))
            {
                m_Offset++;
            }
            return ViewSince(start);
        }

        /// @brief 現在の（あるいは指定オフセット先の）文字を読み取らずに確認します。
        /// @param offset
        /// @return
        [[nodiscard]]
        constexpr char Peek(const std::size_t offset = 0) const noexcept
        {
            if (m_Offset + offset >= m_Source.size()) [[unlikely]]
            {
                return '\0';
            }
            return m_Source[m_Offset + offset];
        }

        /// @brief @brief オフセットを指定した数だけ進めます。
        /// @param count
        constexpr void Advance(const std::size_t count = 1) noexcept
        {
            m_Offset = (std::min)(m_Offset + count, m_Source.size());
        }

        /// @brief 指定した開始位置から現在の位置までの文字列ビューを取得します。
        /// @param start_pos
        /// @return
        [[nodiscard]]
        constexpr std::string_view ViewSince(std::size_t start_pos) const noexcept
        {
            if (start_pos >= m_Offset)
            {
                return {};
            }
            return m_Source.substr(start_pos, m_Offset - start_pos);
        }

        /// @brief 現在の位置から指定した文字数分の文字列ビューを取得します。
        /// @param count
        /// @return
        [[nodiscard]]
        constexpr std::string_view ViewNext(std::size_t count) const noexcept
        {
            return m_Source.substr(m_Offset, (std::min)(count, m_Source.size() - m_Offset));
        }

        /// @brief 現在のオフセット位置を取得します。
        /// @return
        [[nodiscard]]
        constexpr std::size_t Tell() const noexcept
        {
            return m_Offset;
        }

        /// @brief ソース文字列全体のサイズを取得します。
        /// @return
        [[nodiscard]]
        constexpr std::size_t TotalSize() const noexcept
        {
            return m_Source.size();
        }

    private:
        /// @brief
        std::string_view m_Source;

        /// @brief
        std::size_t m_Offset;
    };
} // namespace N503::CommandLine::Character
