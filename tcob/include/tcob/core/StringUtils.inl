// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "StringUtils.hpp"

#include <charconv>

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
                if (topBracket == 0) {
                    topBracket = c;
                }
                if (c == topBracket) {
                    ++bracketCount;
                }
                break;
            case ')':
            case ']':
            case '}':
            case '>':
                if (bracketCount > 0) {
                    if ((topBracket == '(' && c == ')') || (topBracket == '[' && c == ']') || (topBracket == '{' && c == '}') || (topBracket == '<' && c == '>')) {
                        --bracketCount;
                        if (bracketCount == 0) {
                            topBracket = 0;
                        }
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
    if constexpr (std::is_convertible_v<T, string>) {
        return string {std::forward<T>(value)};
    } else {
        return std::to_string(value);
    }
}

template <Arithmetic T>
auto to_number(string_view str) -> std::optional<T>
{
    auto const* valueStrData {str.data()};
    auto const  valueStrSize {str.size()};

    T retValue {0};
    auto [p, ec] {std::from_chars(valueStrData, valueStrData + valueStrSize, retValue)};
    if (ec == std::errc {} && p == valueStrData + valueStrSize) {
        return retValue;
    }

    return std::nullopt;
}

auto join(auto&& container, string_view delim) -> string
{
    if (container.empty()) { return ""; }

    string retValue;
    for (auto it {container.cbegin()}; it != container.cend(); ++it) {
        retValue += to_string(*it);
        retValue += delim;
    }

    retValue.erase(retValue.size() - delim.size());

    return retValue;
}
}
