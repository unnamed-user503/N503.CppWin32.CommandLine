#pragma once

// External Project
#include <N503/Abi/Api.hpp>

// C++ Standard Libraries
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

// Platform/Thirdparty Libraries

namespace N503::CommandLine
{

    /// @brief
    class N503_API Arguments final
    {
    public:
        /// @brief
        /// @param title
        /// @param width
        /// @param height
        explicit Arguments();

        /// @brief
        ~Arguments();

        /// @brief
        /// @param
        Arguments(const Arguments&) = delete;

        /// @brief
        /// @param
        /// @return
        auto operator=(const Arguments&) -> Arguments& = delete;

        /// @brief
        /// @param
        Arguments(Arguments&&);

        /// @brief
        /// @param
        /// @return
        auto operator=(Arguments&&) -> Arguments&;

    public:
        /// @brief 
        /// @param index 
        /// @return 
        auto GetShortOption(std::size_t index) const -> std::string_view;

        /// @brief 
        /// @return 
        auto GetShortOptionCount() const -> std::size_t;

        /// @brief 
        /// @param name 
        /// @return 
        auto GetOption(std::string_view name) const -> std::string_view;

        /// @brief 
        /// @param name 
        /// @return 
        auto GetProperty(std::string_view name) const -> std::string_view;

        /// @brief 
        /// @param index 
        /// @return 
        auto GetArgument(std::size_t index) const -> std::string_view;

        /// @brief 
        /// @return 
        auto GetArgumentCount() const -> std::size_t;

    public:
        /// @brief 実装の詳細を隠蔽するための不透明な構造体。
        struct Entity;

#ifdef N503_DLL_EXPORTS
        /// @brief 内部の実装実体（Entity）への参照を取得します。
        /// @note このメソッドはライブラリ内部（DLL境界の内側）でのみ使用されます。
        /// @return Entity を管理する unique_ptr への参照。
        [[nodiscard]]
        auto GetEntity() -> std::unique_ptr<Entity>&
        {
            return m_Entity;
        }
#endif

    private:
#pragma warning(push)
#pragma warning(disable : 4251) // DLL境界を越える際の unique_ptr に関する警告を抑制
        /// @brief
        std::unique_ptr<Entity> m_Entity;
#pragma warning(pop)
    };

    using Args = Arguments;

} // namespace N503::CommandLine
