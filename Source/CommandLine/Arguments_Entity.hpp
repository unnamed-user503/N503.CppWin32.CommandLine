#pragma once

#include <N503/CommandLine/Arguments.hpp>
#include <N503/Memory/Storage/Arena.hpp>
#include <map>
#include <string_view>
#include <vector>

namespace N503::CommandLine
{

    struct Arguments::Entity
    {
        N503::Memory::Storage::Arena Arena{ 256 };

        std::map<std::string_view, std::string_view> Options{};

        std::vector<std::string_view> ShortOptions{};

        std::map<std::string_view, std::string_view> Properties{};

        std::vector<std::string_view> Arguments{};
    };

} // namespace N503::CommandLine
