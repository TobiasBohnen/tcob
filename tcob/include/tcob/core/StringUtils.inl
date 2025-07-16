// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "StringUtils.hpp"

#include <charconv>
#include <optional>
#include <system_error>

namespace tcob::helper {

auto split_for_each(string_view str, char delim, auto&& f) -> bool
{
    usize start {0};
    usize end {str.find(delim)};
    while (end != string_view::npos) {
        if (!f(str.substr(start, end - start))) { return false; }

        start = end + 1;
        end   = str.find(delim, start);
    }
    if (start < str.size()) {
        if (!f(str.substr(start))) { return false; }
    }

    return true;
}

auto split_preserve_brackets_for_each(string_view str, char delim, auto&& f) -> bool
{
    bool inQuote {false};

    char topBracket {0};
    i32  bracketCount {0};

    usize start {0};
    for (usize i {0}; i < str.size(); ++i) {
        char const c {str[i]};
        if (c == '"') {
            inQuote = !inQuote;
        } else {
            switch (c) {
            case '(':
            case '[':
            case '{':
            case '<':
                if (topBracket == 0) { topBracket = c; }
                if (c == topBracket) { ++bracketCount; }
                break;
            case ')':
            case ']':
            case '}':
            case '>':
                if (bracketCount > 0) {
                    if ((topBracket == '(' && c == ')')
                        || (topBracket == '[' && c == ']')
                        || (topBracket == '{' && c == '}')
                        || (topBracket == '<' && c == '>')) {
                        --bracketCount;
                        if (bracketCount == 0) { topBracket = 0; }
                    }
                }
                break;
            default:
                break;
            }
        }

        if (c == delim && !inQuote && bracketCount == 0) {
            if (start <= i) {
                if (!f(str.substr(start, i - start))) {
                    return false;
                }
            }
            start = i + 1;
        }
    }

    if (start <= str.size()) {
        if (!f(str.substr(start))) {
            return false;
        }
    }

    return true;
}

template <typename T>
auto to_string(T&& value) -> string
{
    if constexpr (std::is_convertible_v<T, string> || std::is_convertible_v<T, string_view>) {
        return string {std::forward<T>(value)};
    } else {
        return std::to_string(value);
    }
}

template <Arithmetic T>
auto to_number(string_view str) -> std::optional<T>
{
#if defined(__cpp_lib_to_chars)
    auto const* valueStrData {str.data()};
    auto const  valueStrSize {str.size()};

    T retValue {0};
    auto [p, ec] {std::from_chars(valueStrData, valueStrData + valueStrSize, retValue)};
    if (ec == std::errc {} && p == valueStrData + valueStrSize) {
        return retValue;
    }

    return std::nullopt;
#else
    if (str.empty() || str[0] == '\0') { return std::nullopt; }
    string temp {str};
    if constexpr (Integral<T>) {
        char*     end {nullptr};
        i64 const result {std::strtol(temp.c_str(), &end, 10)};
        if (*end == '\0' && errno != ERANGE) {
            return static_cast<T>(result);
        }
        return std::nullopt;
    } else if constexpr (FloatingPoint<T>) {
        char*     end {nullptr};
        f64 const result {std::strtod(temp.c_str(), &end)};
        if (*end == '\0' && errno != ERANGE) {
            return static_cast<T>(result);
        }
        return std::nullopt;
    }
#endif
}

auto join(auto&& container, string_view delim) -> string
{
    auto       it {std::begin(container)};
    auto const last {std::end(container)};
    if (it == last) { return {}; }

    string result;
    result += to_string(*it);
    ++it;

    for (; it != last; ++it) {
        result += delim;
        result += to_string(*it);
    }

    return result;
}
}
