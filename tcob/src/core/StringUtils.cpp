// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/StringUtils.hpp"

#include <algorithm>
#include <cctype>

#include <utf8/unchecked.h>

#include "tcob/core/random/Random.hpp"

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

auto find_unquoted(string_view source, char needle) -> string_view::size_type
{
    char const quote {source[0]};
    bool       inQuotes {false};
    if (quote != '"' && quote != '\'') {
        return source.find(needle);
    }

    for (usize i {0}; i < source.size(); ++i) {
        char const c {source[i]};
        if (c == quote) {
            inQuotes = !inQuotes;
        } else if (!inQuotes && c == needle) {
            return i;
        }
    }

    return string_view::npos;
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

auto case_insensitive_contains(string_view lhs, string_view rhs) -> bool
{
    return std::search(
               lhs.begin(), lhs.end(),
               rhs.begin(), rhs.end(),
               [](char a, char b) { return std::toupper(a) == std::toupper(b); })
        != lhs.end();
}

auto wildcard_match(string_view str, string_view pattern) -> bool
{
    auto strIt {str.begin()};
    auto patternIt {pattern.begin()};

    auto strLastStar {str.end()};
    auto patternLastStar {pattern.end()};

    while (strIt != str.end()) {
        if (patternIt != pattern.end() && (*patternIt == '?' || *patternIt == *strIt)) { // match
            ++strIt;
            ++patternIt;
        } else if (patternIt != pattern.end() && *patternIt == '*') {                    // begin star matching
            patternLastStar = patternIt++;
            strLastStar     = strIt;
        } else if (patternLastStar != pattern.end()) {                                   // continue star matching
            patternIt = patternLastStar + 1;
            strIt     = ++strLastStar;
        } else {
            return false;
        }
    }

    while (patternIt != pattern.end() && *patternIt == '*') {
        ++patternIt;
    }

    return patternIt == pattern.end();
}

auto random_string(usize length) -> string
{
    std::string static const characters {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
    rng static rand;

    std::string retValue;
    for (usize i {0}; i < length; ++i) {
        retValue += characters[rand(isize {0}, std::ssize(characters) - 1)];
    }

    return retValue;
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
