// Copyright (c) 2025 Tobias Bohnen
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

auto split(string_view str, char delim) -> std::vector<string_view>
{
    std::vector<string_view> retValue;
    retValue.reserve(10);

    usize start {0};
    usize end {str.find(delim)};
    while (end != string_view::npos) {
        retValue.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end   = str.find(delim, start);
    }
    if (start < str.size()) {
        retValue.emplace_back(str.substr(start));
    }
    return retValue;
}

auto split_once(string_view str, char delim) -> std::pair<string_view, string_view>
{
    usize const pos {str.find(delim)};
    if (pos == string_view::npos) { return {str, ""}; }

    string_view key {str.substr(0, pos)};
    string_view value {str.substr(pos + 1)};
    return {key, value};
}

auto split_preserve_brackets(string_view str, char delim) -> std::vector<string_view>
{
    std::vector<string_view> retValue;
    retValue.reserve(10);
    split_preserve_brackets_for_each(str, delim, [&retValue](string_view token) {
        retValue.emplace_back(token);
        return true;
    });
    return retValue;
}

auto trim(string_view source) -> string_view
{
    auto front {source.find_first_not_of(" \n\r\t")};
    if (front == string_view::npos) { return ""; }

    auto back {source.find_last_not_of(" \n\r\t")};
    back = back == string_view::npos ? source.size() - 1 : back;
    return source.substr(front, back - front + 1);
}

auto replace(string_view source, string_view from, string_view to) -> string
{
    string retValue;
    usize  startPos {0};
    usize  fromPos {0};
    retValue.reserve(source.size());

    while ((fromPos = source.find(from, startPos)) != string_view::npos) {
        retValue.append(source.substr(startPos, fromPos - startPos));
        retValue.append(to);
        startPos = fromPos + from.size();
    }

    retValue.append(source.substr(startPos));

    return retValue;
}

auto to_lower(string_view source) -> string
{
    string retValue {source};
    std::ranges::transform(retValue, retValue.begin(), [](char c) { return std::tolower(c); });
    return retValue;
}

auto case_insensitive_equals(string_view lhs, string_view rhs) -> bool
{
    return std::ranges::equal(lhs, rhs,
                              [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

auto case_insensitive_contains(string_view lhs, string_view rhs) -> bool
{
    return std::search( // NOLINT(modernize-use-ranges)
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
        retValue += characters[rand(usize {0}, characters.size() - 1)];
    }

    return retValue;
}

}

namespace tcob::utf8 {

auto length(utf8_string_view str) -> usize
{
    return static_cast<usize>(::utf8::unchecked::distance(str.begin(), str.end()));
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
