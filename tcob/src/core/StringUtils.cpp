// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/StringUtils.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <utf8/utf8.h>

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
    if (pos == string_view::npos) { return {str, {}}; }

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
    if (front == string_view::npos) { return {}; }

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

auto wildcard_match(string_view str, string_view pattern) -> bool
{
    usize s {0};
    usize p {0};

    usize lastStarS {string_view::npos};
    usize lastStarP {string_view::npos};

    while (s < str.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
            ++s;
            ++p;
        } else if (p < pattern.size() && pattern[p] == '*') {
            lastStarP = p++;
            lastStarS = s;
        } else if (lastStarP != string_view::npos) {
            p = lastStarP + 1;
            s = ++lastStarS;
        } else {
            return false;
        }
    }

    while (p < pattern.size() && pattern[p] == '*') { ++p; }

    return p == pattern.size();
}

auto random_string(usize length) -> string
{
    static string const characters {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
    rng static rand;

    string retValue;
    for (usize i {0}; i < length; ++i) {
        retValue += characters[rand(usize {0}, characters.size() - 1)];
    }

    return retValue;
}

auto join(string_view c, usize count, string_view delim) -> string
{
    if (count == 0 || c.empty()) { return {}; }

    string result;
    result.reserve((c.size() * count) + (delim.size() * (count - 1)));

    for (usize i {0}; i < count; ++i) {
        if (i > 0) {
            result += delim;
        }
        result += c;
    }

    return result;
}

}

namespace tcob::utf8 {

auto length(utf8_string_view str) -> isize
{
    return static_cast<isize>(::utf8::length(str));
}

auto insert(utf8_string_view str, utf8_string_view what, usize pos) -> utf8_string
{
    string retValue {str};

    auto it {retValue.begin()};
    for (usize i {0}; i < pos; ++i) {
        ::utf8::next(it, retValue.end());
    }

    retValue.insert(it, what.begin(), what.end());

    return retValue;
}

auto remove(utf8_string_view str, usize pos, usize count) -> utf8_string
{
    string retValue {str};

    auto start {retValue.begin()};
    for (usize i {0}; i < pos; ++i) {
        ::utf8::next(start, retValue.end());
    }
    auto end {start};
    for (usize i {0}; i < count; ++i) {
        ::utf8::next(end, retValue.end());
    }

    retValue.erase(start, end);

    return retValue;
}

auto substr(utf8_string_view str, usize pos, usize count) -> utf8_string
{
    auto start {str.begin()};
    for (usize i {0}; i < pos; ++i) {
        ::utf8::next_ex(start, str.end());
    }
    auto end {start};
    for (usize i {0}; i < count; ++i) {
        ::utf8::next_ex(end, str.end());
    }

    return {start, end};
}

auto to_lower(utf8_string_view str) -> utf8_string
{
    return ::utf8::tolower({str.data(), str.size()});
}

auto to_upper(utf8_string_view str) -> utf8_string
{
    return ::utf8::toupper({str.data(), str.size()});
}

auto capitalize(utf8_string_view str) -> utf8_string
{
    utf8_string retValue;

    usize const len {::utf8::length(str)};
    auto        it {str.begin()};
    bool        newWord {true};

    for (usize i {0}; i < len; ++i) {
        if (::utf8::isspace(*it)) {
            newWord = true;
            retValue += utf8::substr(str, i, 1);
        } else if (newWord) {
            newWord = false;
            retValue += utf8::to_upper(utf8::substr(str, i, 1));
        } else {
            retValue += utf8::to_lower(utf8::substr(str, i, 1));
        }
        ::utf8::next_ex(it, str.end());
    }

    return retValue;
}

auto to_utf32(utf8_string_view str) -> std::u32string
{
    return ::utf8::runes(str.data(), str.size());
}

}

////////////////////////////////////////////////////////////
