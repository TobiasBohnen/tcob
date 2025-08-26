// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include "tcob/tcob_config.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "tcob/core/Concepts.hpp"

namespace tcob::helper {

TCOB_API auto get_levenshtein_distance(string_view lhs, string_view rhs) -> u32;

template <typename T>
auto          to_string(T&& value) -> string;
TCOB_API auto to_string(char const* s) -> string;

template <Arithmetic T>
auto to_number(string_view str) -> std::optional<T>;

auto          join(auto&& container, string_view delim) -> string;
TCOB_API auto join(string_view c, usize count, string_view delim) -> string;

auto          split_for_each(string_view str, char delim, auto&& f) -> bool;
TCOB_API auto split(string_view str, char delim) -> std::vector<string_view>;
TCOB_API auto split_once(string_view str, char delim) -> std::pair<string_view, string_view>;

auto          split_preserve_brackets_for_each(string_view str, char delim, auto&& f) -> bool;
TCOB_API auto split_preserve_brackets(string_view str, char delim) -> std::vector<string_view>;

TCOB_API auto trim(string_view source) -> string_view;

TCOB_API auto replace(string_view source, string_view from, string_view to) -> string;

TCOB_API auto to_lower(string_view source) -> string;

TCOB_API auto wildcard_match(string_view str, string_view pattern) -> bool;

TCOB_API auto random_string(usize length) -> string;

}

namespace tcob::utf8 {

TCOB_API auto length(utf8_string_view str) -> isize;
TCOB_API auto insert(utf8_string_view str, utf8_string_view what, usize pos) -> utf8_string;
TCOB_API auto remove(utf8_string_view str, usize pos, usize count = 1) -> utf8_string;
TCOB_API auto substr(utf8_string_view str, usize pos, usize count = 1) -> utf8_string;
TCOB_API auto to_lower(utf8_string_view str) -> utf8_string;
TCOB_API auto to_upper(utf8_string_view str) -> utf8_string;
TCOB_API auto capitalize(utf8_string_view str) -> utf8_string;

TCOB_API auto to_utf32(utf8_string_view str) -> std::u32string;

}

#include "StringUtils.inl"
