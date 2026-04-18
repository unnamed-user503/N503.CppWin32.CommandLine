#include "Pch.hpp"
#include "Arguments_Entity.hpp"
#include <N503/CommandLine/Arguments.hpp>

#include <N503/Diagnostics/Sink.hpp>
#include <N503/Syntax/Grammar/Definition.hpp>
#include <N503/Syntax/Grammar/Identifier.hpp>
#include <N503/Syntax/Grammar/Lexeme.hpp>
#include <N503/Syntax/Grammar/Number.hpp>
#include <N503/Syntax/Grammar/String.hpp>
#include <N503/Syntax/Lexing/AnyOf.hpp>
#include <N503/Syntax/Lexing/IsAlnum.hpp>
#include <N503/Syntax/Lexing/IsQuote.hpp>
#include <N503/Syntax/Lexing/IsWhitespace.hpp>
#include <N503/Syntax/Lexing/Lexer.hpp>
#include <N503/Syntax/Lexing/Not.hpp>
#include <N503/Syntax/Node.hpp>
#include <N503/Syntax/NodeType.hpp>
#include <N503/Syntax/NodeVisitor.hpp>
#include <N503/Syntax/Parsing/Parser.hpp>
#include <N503/Syntax/Scanning/DefaultScanner.hpp>
#include <N503/Syntax/Scanning/Identifier.hpp>
#include <N503/Syntax/Scanning/Ignore.hpp>
#include <N503/Syntax/Scanning/Number.hpp>
#include <N503/Syntax/Scanning/String.hpp>
#include <N503/Syntax/Token.hpp>
#include <N503/Syntax/TokenType.hpp>
#include <Windows.h>
#include <iostream>
#include <map>
#include <memory>
#include <string_view>
#include <vector>
#include <string>

namespace N503::CommandLine
{

    namespace Details
    {
        using namespace N503::Syntax;

        // 演算子（記号）の判定：英数字、空白、引用符以外
        using OperatorCondition = Lexing::Not<Lexing::AnyOf<Lexing::IsAlnum, Lexing::IsWhitespace, Lexing::IsQuote>>;
    } // namespace Details

    namespace Scanners
    {
        using namespace N503::Syntax::Scanning;
        using namespace Details;

        // パーサー標準装備のScanning::Operatorは1文字ずつ抽出するので、連続した記号を受け取れるスキャナを自前で用意する
        using Operator = DefaultScanner<TokenType::Operator, OperatorCondition, OperatorCondition, Lexing::Not<OperatorCondition>>;
    } // namespace Scanners

    namespace Grammar
    {
        using namespace N503::Syntax;
        using namespace N503::Syntax::Grammar;

        // 最も基礎となる「値」の定義
        inline constexpr auto BaseValue = (Identifier + *(Lexeme<"."> + Identifier));

        // AtomicValue を先に定義
        inline constexpr auto AtomicValue = (String | BaseValue | Number | Identifier);

        // オプション形式の定義 (AtomicValue を使用)
        inline constexpr auto LongOption = (Lexeme<"--"> + Identifier + Lexeme<"="> + AtomicValue).As<NodeType::LongOption>();
        inline constexpr auto ShortOption = (Lexeme<"-"> + Identifier).As<NodeType::ShortOption>();

        // 代入形式と位置引数の定義
        inline constexpr auto Property = (Identifier + Lexeme<"="> + AtomicValue).As<NodeType::Property>();
        inline constexpr auto PositionalItem = (Property | AtomicValue);

        // グループ化とルート定義
        inline constexpr auto Options = *(LongOption | ShortOption);
        inline constexpr auto PositionalGroup = (*PositionalItem).As<NodeType::PositionalGroup>();

        // 最終的な Root (Options と PositionalGroup を連結)
        inline constexpr auto Root = (*(LongOption | ShortOption | Property | PositionalGroup)).As<NodeType::Root>();
    } // namespace Grammar

    Arguments::Arguments() : m_Entity{ std::make_unique<Arguments::Entity>() }
    {
        using namespace N503::Syntax;

        // Lexerの構成
        // clang-format off
        using Lexer = Lexing::Lexer<
            Scanning::String,     // 引用符で囲まれた文字列
            Scanning::Ignore,     // 空白や改行を無視
            Scanning::Identifier, // 識別子
            Scanning::Number,     // 数字
            Scanners::Operator    // 記号 (CommandLine用にカスタマイズしているCommandLine::"Scanners"::Operator)
        >;
        // clang-format on
        Lexer lexer;

        // 定義した文法を Parser にセット
        using CommandLineRule = Grammar::RuleOf<Grammar::Root>;
        Parsing::Parser<CommandLineRule> parser;

        // 解析実行
        std::string_view source = GetCommandLineA(); // R"("App.exe" -abc -a -b -c --file="Test.dat" root=name foo bar hoge)";
        N503::Diagnostics::Sink sink;

        auto tokens = lexer.Tokenize(source, sink);
        Node* root = parser.Parse(tokens, m_Entity->Arena, sink);

        // コマンド解析結果の成果を得る
        Syntax::NodeVisitor<Arguments> visitor;

        // --- 各ノードに対する処理を登録 ---

        // LongOption: [--][key][=][value] の構造を想定
        visitor.On(NodeType::LongOption, [](Node* node, Arguments& context)
        {
            auto children = node->GetChildren();
            if (children.size() >= 4)
            {
                auto key = children[1]->GetToken().Lexeme;   // "filename1"
                auto value = children[3]->GetToken().Lexeme; // "abc.txt"
                context.m_Entity->Options[key] = value;
            }
        });

        // ShortOption: [-][f] の構造を想定
        visitor.On(NodeType::ShortOption, [](Node* node, Arguments& context)
        {
            auto children = node->GetChildren();
            if (children.size() >= 2)
            {
                context.m_Entity->ShortOptions.push_back(children[1]->GetToken().Lexeme); // "a", "b", "c"
            }
        });

        // Property: [key][=][value] の構造
        visitor.On(NodeType::Property, [](Node* node, Arguments& context)
        {
            auto children = node->GetChildren();
            if (children.size() >= 3)
            {
                auto key = children[0]->GetToken().Lexeme;   // "root"
                auto value = children[2]->GetToken().Lexeme; // "\"abc\""
                context.m_Entity->Properties[key] = value;
            }
        });

        // PositionalGroup 内の Terminal (単なる値)
        // ※ Visitor が再帰的に Visit するため、NodeType::Terminal でも判定可能
        visitor.On(NodeType::Terminal, [](Node* node, Arguments& context)
        {
            // 親が PositionalGroup の場合のみ、位置引数として扱う
            if (node->GetParent() && node->GetParent()->GetType() == NodeType::PositionalGroup)
            {
                context.m_Entity->Arguments.push_back(node->GetToken().Lexeme); // "foo", "bar"
            }
        });

#ifdef _DEBUG
        visitor.On(NodeType::Root, [](Node* node, Arguments& context)
        {
            std::cout << "Entering Root Node..." << std::endl;
        });

        visitor.On(NodeType::PositionalGroup, [](Node* node, Arguments& context)
        {
            std::cout << "Entering PositionalGroup Node..." << static_cast<int>(node->GetType()) << std::endl;
        });
#endif

        // 解析結果を元にメンバ変数にコマンド引数を構築
        visitor.Visit(root, *this);
    }

    auto Arguments::GetShortOption(std::size_t index) const -> std::string_view
    {
        // インデックスが範囲外の場合は空のビューを返す
        if (index >= m_Entity->ShortOptions.size())
        {
            return {};
        }

        return m_Entity->ShortOptions.at(index);
    }

    auto Arguments::GetShortOptionCount() const -> std::size_t
    {
        return m_Entity->ShortOptions.size();
    }

    auto Arguments::GetOption(std::string_view name) const -> std::string_view
    {
        auto it = m_Entity->Options.find(std::string(name));
        if (it == m_Entity->Options.end())
        {
            return {};
        }

        return it->second;
    }

    auto Arguments::GetProperty(std::string_view name) const -> std::string_view
    {
        auto it = m_Entity->Properties.find(std::string(name));
        if (it == m_Entity->Properties.end())
        {
            return {};
        }

        return it->second;
    }

    auto Arguments::GetArgument(std::size_t index) const -> std::string_view
    {
        if (index >= m_Entity->Arguments.size())
        {
            return {};
        }

        return m_Entity->Arguments.at(index);
    }

    auto Arguments::GetArgumentCount() const -> std::size_t
    {
        return m_Entity->Arguments.size();
    }

    Arguments::~Arguments() = default;

    Arguments::Arguments(Arguments&&) = default;

    auto Arguments::operator=(Arguments&&) -> Arguments& = default;

} // namespace N503::CommandLine
