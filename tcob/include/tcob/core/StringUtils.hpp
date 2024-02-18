// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <vector>

#include "tcob/core/Concepts.hpp"

namespace tcob::helper {

TCOB_API auto get_levenshtein_distance(string_view lhs, string_view rhs) -> u32;

template <typename T>
auto          to_string(T&& value) -> string;
TCOB_API auto to_string(char const* s) -> string;

auto join(auto&& container, string_view delim) -> string;

auto          split_for_each(string_view str, char delim, auto&& f) -> bool;
TCOB_API auto split(string_view str, char delim) -> std::vector<string>;
TCOB_API void split_to(string_view str, char delim, std::vector<string>& out);

auto          split_preserve_brackets_for_each(string_view str, char delim, auto&& f) -> bool;
TCOB_API auto split_preserve_brackets(string_view str, char delim) -> std::vector<string>;
TCOB_API void split_preserve_brackets_to(string_view str, char delim, std::vector<string>& out);

TCOB_API auto trim(string_view source) -> string_view;

TCOB_API auto to_lower(string_view source) -> string;

TCOB_API auto case_insensitive_equals(string_view lhs, string_view rhs) -> bool;

TCOB_API auto wildcard_match(string_view str, string_view pattern) -> bool;
}

namespace tcob::utf8 {

TCOB_API auto length(utf8_string_view str) -> usize;
TCOB_API auto insert(utf8_string_view str, utf8_string_view what, usize pos) -> utf8_string;
TCOB_API auto remove(utf8_string_view str, usize pos, usize count = 1) -> utf8_string;
TCOB_API auto substr(utf8_string_view str, usize pos, usize count = 1) -> utf8_string;

}

////////////////////////////////////////////////////////////

namespace tcob::detail {

struct case_insensitive_hash { // for std::unordered_map
    auto operator()(string const& s) const -> usize
    {
        return std::hash<string> {}(helper::to_lower(s));
    }
};

struct case_insensitive_equal { // for std::unordered_map
    auto operator()(string const& lhs, string const& rhs) const -> bool
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char lhc, char rhc) {
            return std::tolower(lhc) == std::tolower(rhc);
        });
    }
};

struct case_insensitive_compare { // for std::map
    auto operator()(string const& lhs, string const& rhs) const -> bool
    {
        return helper::to_lower(lhs) < helper::to_lower(rhs);
    }
};

}

#include "StringUtils.inl"
