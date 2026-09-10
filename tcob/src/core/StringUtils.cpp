// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/StringUtils.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

#include <utf8proc.h>

#include "tcob/core/random/Random.hpp"

namespace tcob::helper {

auto levenshtein_distance(string_view lhs, string_view rhs) -> u32
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

auto is_ascii_alpha(char c) -> bool { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

auto split(string_view str, char delim) -> std::vector<string_view>
{
    return split(str, {&delim, 1});
}

auto split(string_view str, string_view delim) -> std::vector<string_view>
{
    std::vector<string_view> retValue;
    retValue.reserve(10);

    usize start {0};
    usize end {str.find(delim)};
    while (end != string_view::npos) {
        retValue.emplace_back(str.substr(start, end - start));
        start = end + delim.size();
        end   = str.find(delim, start);
    }
    if (start < str.size()) {
        retValue.emplace_back(str.substr(start));
    }
    return retValue;
}

auto split_once(string_view str, char delim) -> std::pair<string_view, string_view>
{
    return split_once(str, {&delim, 1});
}

auto split_once(string_view str, string_view delim) -> std::pair<string_view, string_view>
{
    usize const pos {str.find(delim)};
    if (pos == string_view::npos) { return {str, {}}; }

    string_view key {str.substr(0, pos)};
    string_view value {str.substr(pos + delim.size())};
    return {key, value};
}

auto split_preserve_brackets(string_view str, char delim) -> std::vector<string_view>
{
    std::vector<string_view> retValue;
    retValue.reserve(10);
    split_preserve_brackets(str, delim, [&retValue](string_view token) {
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
    if (from.empty()) { return string {source}; }

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

auto rep(string_view c, usize count, string_view delim) -> string
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

static auto advance_codepoints(char const*& ptr, char const* end, usize n) -> void
{
    while (n > 0 && ptr < end) {
        utf8proc_int32_t       cp {};
        utf8proc_ssize_t const len {utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t const*>(ptr), end - ptr, &cp)};
        if (len <= 0) { break; }
        ptr += len;
        --n;
    }
}

static auto is_space_cp(utf8proc_int32_t cp) -> bool
{
    switch (cp) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\f':
    case '\v':
        return true;
    default:
        return utf8proc_category(cp) == UTF8PROC_CATEGORY_ZS;
    }
}

static auto encode_append(utf8_string& out, utf8proc_int32_t cp) -> void
{
    std::array<utf8proc_uint8_t, 4> buf {};
    utf8proc_ssize_t                len {utf8proc_encode_char(cp, buf.data())};
    if (len > 0) {
        out.append(reinterpret_cast<char const*>(buf.data()), static_cast<usize>(len));
    }
}

auto length(utf8_string_view str) -> isize
{
    isize            count {0};
    auto const*      ptr {str.data()};
    char const*      end {str.data() + str.size()};
    utf8proc_int32_t cp {};

    while (ptr < end) {
        utf8proc_ssize_t const len {utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t const*>(ptr), end - ptr, &cp)};
        if (len <= 0) { break; }
        ptr += len;
        ++count;
    }

    return count;
}

auto insert(utf8_string_view str, utf8_string_view what, usize pos) -> utf8_string
{
    utf8_string retValue {str};

    char const* ptr {retValue.data()};
    char const* end {retValue.data() + retValue.size()};
    advance_codepoints(ptr, end, pos);

    usize const byteOffset {static_cast<usize>(ptr - retValue.data())};
    retValue.insert(byteOffset, what.data(), what.size());

    return retValue;
}

auto remove(utf8_string_view str, usize pos, usize count) -> utf8_string
{
    utf8_string retValue {str};

    char const* base {retValue.data()};
    char const* end {retValue.data() + retValue.size()};

    char const* start {base};
    advance_codepoints(start, end, pos);

    char const* stop {start};
    advance_codepoints(stop, end, count);

    usize startByte {static_cast<usize>(start - base)};
    usize stopByte {static_cast<usize>(stop - base)};
    retValue.erase(startByte, stopByte - startByte);

    return retValue;
}

auto substr(utf8_string_view str, usize pos, usize count) -> utf8_string
{
    char const* base {str.data()};
    char const* end {str.data() + str.size()};

    char const* start {base};
    advance_codepoints(start, end, pos);

    char const* stop {start};
    advance_codepoints(stop, end, count);

    return utf8_string {start, static_cast<usize>(stop - start)};
}

auto to_lower(utf8_string_view str) -> utf8_string
{
    utf8_string retValue;
    retValue.reserve(str.size());

    char const*      ptr {str.data()};
    char const*      end {str.data() + str.size()};
    utf8proc_int32_t cp {};

    while (ptr < end) {
        utf8proc_ssize_t const len {utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t const*>(ptr), end - ptr, &cp)};
        if (len <= 0) { break; }
        ptr += len;
        encode_append(retValue, utf8proc_tolower(cp));
    }

    return retValue;
}

auto to_upper(utf8_string_view str) -> utf8_string
{
    utf8_string retValue;
    retValue.reserve(str.size());

    char const*      ptr {str.data()};
    char const*      end {str.data() + str.size()};
    utf8proc_int32_t cp {};

    while (ptr < end) {
        utf8proc_ssize_t const len {utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t const*>(ptr), end - ptr, &cp)};
        if (len <= 0) { break; }
        ptr += len;
        encode_append(retValue, utf8proc_toupper(cp));
    }

    return retValue;
}

auto capitalize(utf8_string_view str) -> utf8_string
{
    utf8_string retValue;
    retValue.reserve(str.size());

    char const*      ptr {str.data()};
    char const*      end {str.data() + str.size()};
    utf8proc_int32_t cp {};
    bool             newWord {true};

    while (ptr < end) {
        utf8proc_ssize_t const len {utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t const*>(ptr), end - ptr, &cp)};
        if (len <= 0) { break; }
        ptr += len;

        if (is_space_cp(cp)) {
            newWord = true;
            encode_append(retValue, cp);
        } else if (newWord) {
            newWord = false;
            encode_append(retValue, utf8proc_totitle(cp));
        } else {
            encode_append(retValue, utf8proc_tolower(cp));
        }
    }

    return retValue;
}

auto to_utf32(utf8_string_view str) -> std::u32string
{
    std::u32string retValue;
    retValue.reserve(str.size());

    char const*      ptr {str.data()};
    char const*      end {str.data() + str.size()};
    utf8proc_int32_t cp {};

    while (ptr < end) {
        utf8proc_ssize_t const len {utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t const*>(ptr), end - ptr, &cp)};
        if (len <= 0) { break; }
        ptr += len;
        retValue.push_back(static_cast<char32_t>(cp));
    }

    return retValue;
}
}

////////////////////////////////////////////////////////////
