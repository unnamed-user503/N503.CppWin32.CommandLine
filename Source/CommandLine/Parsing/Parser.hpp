#pragma once

#include "../Lexing/Scanner.hpp"
#include <string>
#include <utility>
#include <vector>

namespace N503::CommandLine::Parsing
{
    /**
     * @brief Scanner から得られるトークン列を解析し、引数リストを構築するクラス。
     */
    class Parser final
    {
    public:
        explicit Parser(Lexing::Scanner& scanner) : m_Scanner(scanner)
        {
        }

        /**
         * @brief 全てのトークンを解析し、引数のベクトルを返します。
         */
        std::vector<std::string> Parse()
        {
            std::vector<std::string> args;
            std::string currentArg;
            bool hasContent = false;

            while (auto token = m_Scanner.Next())
            {
                using Bits = Lexing::Token::Bits;

                // 1. 引数の区切り（デリミタ）の処理
                if (token.code == Bits::Delimiter || token.code == Bits::ApplicationNameDelimiter)
                {
                    if (hasContent)
                    {
                        args.push_back(std::move(currentArg));
                        currentArg.clear();
                        hasContent = false;
                    }
                    continue;
                }

                // 2. 特殊なエスケープシーケンス（\" や ""）の処理
                if (token.code == Bits::EscapeSequence)
                {
                    // ビューから適切な文字を抽出（\" なら "、"" なら "）
                    if (token.view == "\"\"" || token.view == "\\\"")
                    {
                        currentArg += '"';
                    }
                    else
                    {
                        currentArg += token.view;
                    }
                    hasContent = true;
                    continue;
                }

                // 3. バックスラッシュの処理（MSVC規則の適用）
                if (token.code == Bits::BackslashCloseForOdd)
                {
                    // 奇数個の \ + " の場合： (n-1)/2 個の \ を追加し、" をリテラルとして扱う
                    // ※ Scanner 側で最後の \ と " が消費されていることを前提
                    appendBackslashes(currentArg, token.view.size() / 2);
                    currentArg += '"';
                    hasContent = true;
                }
                else if (token.code == Bits::BackslashCloseForEven)
                {
                    // 偶数個の \ + " の場合： n/2 個の \ を追加し、" は区切りとして扱う
                    appendBackslashes(currentArg, token.view.size() / 2);
                    hasContent = true;
                    // " 自体は Scanner 側で次の DoubleQuotationStart 等として処理されるためここでは足さない
                }
                else if (token.code == Bits::Backslash || token.code == Bits::BackslashClose)
                {
                    // 引用符が絡まないバックスラッシュはそのままリテラル
                    currentArg += token.view;
                    hasContent = true;
                }

                // 4. 通常のリテラル文字
                else if (token.code == Bits::Literal)
                {
                    currentArg += token.view;
                    hasContent = true;
                }

                // 5. 引用符の開始・終了（内容は Literal として流れてくるため、フラグのみ確認）
                else if (token.code == Bits::DoubleQuotationStart || token.code == Bits::DoubleQuotationClose)
                {
                    // 空の引用符 "" があった場合に引数として成立させるため
                    hasContent = true;
                }
            }

            // 最後の引数を追加
            if (hasContent)
            {
                args.push_back(std::move(currentArg));
            }

            return args;
        }

    private:
        Lexing::Scanner& m_Scanner;

        static void appendBackslashes(std::string& dest, std::size_t count)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                dest += '\\';
            }
        }
    };
} // namespace N503::CommandLine::Parsing
