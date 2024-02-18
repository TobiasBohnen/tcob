// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/StringUtils.hpp"

#include <algorithm>
#include <cctype>

#include <utf8/unchecked.h>

namespace tcob::helper {

auto get_levenshtein_distance(string_view lhs, string_view rhs) -> u32
{
    if (lhs == rhs) { return 0; }

    u32 const lhsSize {static_cast<u32>(lhs.size())};
    u32 const rhsSize {static_cast<u32>(rhs.size())};

    if (lhsSize == 0) { return rhsSize; }
    if (rhsSize == 0) { return lhsSize; }

    std::vector<u32> v0(rhsSize + 1);
    std::vector<u32> v1(rhsSize + 1);

    for (u32 i {0}; i <= rhsSize; ++i) {
        v0[i] = i;
    }

    for (u32 i {0}; i < lhsSize; ++i) {
        v1[0] = i + 1;

        for (u32 j {0}; j < rhsSize; ++j) {
            v1[j + 1] = std::min({v0[j + 1] + 1, v1[j] + 1, lhs[i] == rhs[j] ? v0[j] : v0[j] + 1});
        }

        std::swap(v0, v1);
    }

    return v0[rhsSize];
}

auto to_string(char const* s) -> string
{
    return s == nullptr ? string {} : s;
}

auto split(string_view str, char delim) -> std::vector<string>
{
    std::vector<string> retValue;
    retValue.reserve(10);
    split_to(str, delim, retValue);
    return retValue;
}

void split_to(string_view str, char delim, std::vector<string>& out)
{
    out.clear();

    usize start {0};
    usize end {str.find(delim)};
    while (end != string_view::npos) {
        out.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end   = str.find(delim, start);
    }
    if (start < str.size()) {
        out.emplace_back(str.substr(start));
    }
}

auto split_preserve_brackets(string_view str, char delim) -> std::vector<string>
{
    std::vector<string> retValue;
    retValue.reserve(10);
    split_preserve_brackets_to(str, delim, retValue);
    return retValue;
}

void split_preserve_brackets_to(string_view str, char delim, std::vector<string>& out)
{
    out.clear();
    split_preserve_brackets_for_each(str, delim, [&out](string_view token) {
        out.emplace_back(token);
        return true;
    });
}

auto trim(string_view source) -> string_view
{
    auto front {source.find_first_not_of(" \n\r\t")};
    if (front == string_view::npos) {
        return "";
    }
    auto back {source.find_last_not_of(" \n\r\t")};
    back = back == string_view::npos ? source.size() - 1 : back;
    return source.substr(front, back - front + 1);
}

auto to_lower(string_view source) -> string
{
    string retValue {source};
    std::transform(retValue.begin(), retValue.end(), retValue.begin(), [](char c) { return std::tolower(c); });
    return retValue;
}

auto case_insensitive_equals(string_view lhs, string_view rhs) -> bool
{
    return std::equal(lhs.begin(), lhs.end(),
                      rhs.begin(), rhs.end(),
                      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

auto wildcard_match(string_view str, string_view pattern) -> bool
{
    if (pattern.empty()) {
        return str.empty();
    }
    if (str.empty()) {
        return pattern.empty() || (pattern.size() == 1 && pattern[0] == '*');
    }

    if (pattern[0] == '*') {
        for (usize s {0}; s <= str.size(); ++s) {
            if (wildcard_match(str.substr(s), pattern.substr(1))) {
                return true;
            }
        }
        return false;
    }

    if (pattern[0] != '?' && pattern[0] != str[0]) {
        return false;
    }

    return wildcard_match(str.substr(1), pattern.substr(1));
}

}

namespace tcob::utf8 {

auto length(utf8_string_view str) -> usize
{
    return ::utf8::unchecked::distance(str.begin(), str.end());
}

auto insert(utf8_string_view str, utf8_string_view what, usize pos) -> utf8_string
{
    string retValue {str};

    auto it {retValue.begin()};
    ::utf8::unchecked::advance(it, pos);

    retValue.insert(it, what.begin(), what.end());

    return retValue;
}

auto remove(utf8_string_view str, usize pos, usize count) -> utf8_string
{
    string retValue {str};

    auto start {retValue.begin()};
    ::utf8::unchecked::advance(start, pos);

    auto end {start};
    ::utf8::unchecked::advance(end, count);

    retValue.erase(start, end);

    return retValue;
}

auto substr(utf8_string_view str, usize pos, usize count) -> utf8_string
{
    auto start {str.begin()};
    ::utf8::unchecked::advance(start, pos);

    auto end {start};
    ::utf8::unchecked::advance(end, count);

    return {start, end};
}

}

////////////////////////////////////////////////////////////
