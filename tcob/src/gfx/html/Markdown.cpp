// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/html/Markdown.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <span>
#include <vector>

#include "tcob/core/io/SpanStream.hpp"

namespace tcob::gfx::html {

namespace detail {

    static void EscapeChar(utf8_string& out, char c)
    {
        switch (c) {
        case '&': out += "&amp;"; return;
        case '<': out += "&lt;"; return;
        case '>': out += "&gt;"; return;
        case '"': out += "&quot;"; return;
        default:  out += c;
        }
    }

    static void EscapeRange(utf8_string& out, utf8_string const& s, usize from, usize to)
    {
        for (usize i {from}; i < to; ++i) { EscapeChar(out, s[i]); }
    }

    static auto IsSpace(char c) -> bool { return c == ' ' || c == '\t'; }

    static auto ParseInline(utf8_string const& s, usize from, usize to) -> utf8_string
    {
        utf8_string out;
        for (usize i {from}; i < to;) {
            char c {s[i]};

            // inline html
            if (c == '<' && i + 1 < to) {
                char const next = {s[i + 1]};
                if (std::isalpha(next) || next == '/' || next == '!' || next == '?') {
                    usize const closePos {s.find('>', i + 1)};
                    if (closePos != utf8_string::npos) {
                        // Include the tag in output without escaping
                        out += s.substr(i, closePos - i + 1);
                        i = closePos + 1;
                        continue;
                    }
                }
            }

            // backslash escape
            if (c == '\\' && i + 1 < to) {
                EscapeChar(out, s[i + 1]);
                i += 2;
                continue;
            }

            // inline code  `...`  or  ``...``
            if (c == '`') {
                usize tickCount {0};
                while (i + tickCount < to && s[i + tickCount] == '`') { ++tickCount; }
                usize const closePos {s.find(utf8_string(tickCount, '`'), i + tickCount)};
                if (closePos != utf8_string::npos && closePos + tickCount <= to) {
                    utf8_string escaped;
                    EscapeRange(escaped, s, i + tickCount, closePos);
                    out += std::format("<code>{}</code>", escaped);
                    i = closePos + tickCount;
                    continue;
                }
                EscapeChar(out, c);
                ++i;
                continue;
            }

            // bold  **...**  or  __...__  (checked before italic)
            if ((c == '*' || c == '_') && i + 1 < to && s[i + 1] == c) {
                // for __, require whitespace before opener
                if (c == '_' && i > from && !IsSpace(s[i - 1])) {
                    EscapeChar(out, c);
                    ++i;
                    continue;
                }
                usize closePos {utf8_string::npos};
                for (usize j {i + 2}; j + 1 < to; ++j) {
                    if (s[j] == c && s[j + 1] == c) {
                        // count full run
                        usize runEnd {j + 2};
                        while (runEnd < to && s[runEnd] == c) { ++runEnd; }
                        usize const runLen {runEnd - j};
                        // for __, require whitespace after closer
                        if (c == '_' && runEnd < to && !IsSpace(s[runEnd])) {
                            j = runEnd - 1;
                            continue;
                        }
                        // use last two chars of run as the close
                        closePos = j + (runLen - 2);
                        break;
                    }
                }
                if (closePos != utf8_string::npos && closePos + 2 <= to) {
                    out += std::format("<strong>{}</strong>", ParseInline(s, i + 2, closePos));
                    i = closePos + 2;
                    continue;
                }
            }

            // italic  *...*  or  _..._
            if (c == '*' || c == '_') {
                // for _, require whitespace before opener
                if (c == '_' && i > from && !IsSpace(s[i - 1])) {
                    EscapeChar(out, c);
                    ++i;
                    continue;
                }
                // find closing single delimiter not part of a ** run
                usize closePos {utf8_string::npos};
                for (usize j {i + 1}; j < to; ++j) {
                    if (s[j] == c) {
                        bool const prevSame {j > 0 && s[j - 1] == c};
                        bool const nextSame {j + 1 < to && s[j + 1] == c};
                        if (prevSame || nextSame) { continue; }
                        // for _, require whitespace after closer
                        if (c == '_' && j + 1 < to && !IsSpace(s[j + 1])) { continue; }
                        closePos = j;
                        break;
                    }
                }
                if (closePos != utf8_string::npos && closePos < to) {
                    out += std::format("<em>{}</em>", ParseInline(s, i + 1, closePos));
                    i = closePos + 1;
                    continue;
                }
                EscapeChar(out, c);
                ++i;
                continue;
            }

            // strikethrough  ~~...~~
            if (c == '~' && i + 1 < to && s[i + 1] == '~') {
                usize const closePos {s.find("~~", i + 2)};
                if (closePos != utf8_string::npos && closePos + 2 <= to) {
                    out += std::format("<del>{}</del>", ParseInline(s, i + 2, closePos));
                    i = closePos + 2;
                    continue;
                }
            }

            // inline color {color}(text)
            if (c == '{') {
                usize const closeBrace {s.find('}', i + 1)};
                if (closeBrace != string::npos && closeBrace + 1 < to && s[closeBrace + 1] == '(') {
                    usize const closeParen {s.find(')', closeBrace + 2)};
                    if (closeParen != string::npos) {
                        string const color {s.substr(i + 1, closeBrace - i - 1)};
                        string const text {ParseInline(s, closeBrace + 2, closeParen)};

                        out += std::format(R"(<span style="color:{}">{}</span>)", color, text);
                        i = closeParen + 1;
                        continue;
                    }
                }
            }

            // image  ![alt](url)
            if (c == '!' && i + 1 < to && s[i + 1] == '[') {
                usize const altEnd {s.find(']', i + 2)};
                if (altEnd != utf8_string::npos && altEnd + 1 < to && s[altEnd + 1] == '(') {
                    usize const urlEnd {s.find(')', altEnd + 2)};
                    if (urlEnd != utf8_string::npos && urlEnd <= to) {
                        utf8_string url, alt;
                        EscapeRange(url, s, altEnd + 2, urlEnd);
                        EscapeRange(alt, s, i + 2, altEnd);
                        out += std::format(R"(<img src="{}" alt="{}">)", url, alt);
                        i = urlEnd + 1;
                        continue;
                    }
                }
            }

            // link  [text](url)
            if (c == '[') {
                usize const textEnd {s.find(']', i + 1)};
                if (textEnd != utf8_string::npos && textEnd + 1 < to && s[textEnd + 1] == '(') {
                    usize const urlEnd {s.find(')', textEnd + 2)};
                    if (urlEnd != utf8_string::npos && urlEnd <= to) {
                        utf8_string url;
                        EscapeRange(url, s, textEnd + 2, urlEnd);
                        out += std::format(R"(<a href="{}">{}</a>)", url, ParseInline(s, i + 1, textEnd));
                        i = urlEnd + 1;
                        continue;
                    }
                }
            }

            EscapeChar(out, c);
            ++i;
        }
        return out;
    }

    static auto LeadingSpaces(utf8_string const& s) -> usize
    {
        usize n {0};
        while (n < s.size() && s[n] == ' ') { ++n; }
        return n;
    }

    // first non-space character, or 0 if line is blank
    static auto FirstChar(utf8_string const& s) -> char
    {
        usize const pos {LeadingSpaces(s)};
        return pos < s.size() ? s[pos] : 0;
    }

    // returns 1-6 for ATX headings, 0 otherwise
    static auto HeadingLevel(utf8_string const& s) -> i32
    {
        usize const start {LeadingSpaces(s)};
        usize       hashes {0};
        while (start + hashes < s.size() && s[start + hashes] == '#') { ++hashes; }
        if (!hashes || hashes > 6 || start + hashes >= s.size() || s[start + hashes] != ' ') { return 0; }
        return static_cast<i32>(hashes);
    }

    static auto IsThematicBreak(utf8_string const& s) -> bool
    {
        char const c {FirstChar(s)};
        if (c != '-' && c != '*' && c != '_') { return false; }
        i32 count {0};
        for (char x : s) {
            if (x == c) {
                ++count;
            } else if (x != ' ') {
                return false;
            }
        }
        return count >= 3;
    }

    static auto IsFencedCodeOpen(utf8_string const& s) -> bool
    {
        usize const pos {LeadingSpaces(s)};
        return pos + 2 < s.size()
            && ((s[pos] == '`' && s[pos + 1] == '`' && s[pos + 2] == '`')
                || (s[pos] == '~' && s[pos + 1] == '~' && s[pos + 2] == '~'));
    }

    static auto IsBulletListItem(utf8_string const& s) -> bool
    {
        usize const pos {LeadingSpaces(s)};
        char const  c {FirstChar(s)};
        return (c == '-' || c == '*' || c == '+') && pos + 1 < s.size() && s[pos + 1] == ' ';
    }

    static auto IsOrderedListItem(utf8_string const& s, i32& startNum) -> bool
    {
        usize i {LeadingSpaces(s)};
        i32   num {0};
        bool  hasDigit {false};
        while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
            num      = (num * 10) + (s[i++] - '0');
            hasDigit = true;
        }
        if (!hasDigit || i >= s.size() || (s[i] != '.' && s[i] != ')')
            || i + 1 >= s.size() || s[i + 1] != ' ') { return false; }
        startNum = num;
        return true;
    }

    static auto IsBlockquoteLine(utf8_string const& s) -> bool { return FirstChar(s) == '>'; }

    static auto AllSameChar(utf8_string const& s, char c) -> bool
    {
        return !s.empty() && std::ranges::all_of(s, [c](char x) { return x == c; });
    }

    auto md::is_eof() const -> bool { return CurrentPos >= Lines.size(); }

    auto md::parse() -> utf8_string
    {
        utf8_string out;
        while (!is_eof()) { out += block(); }
        return out;
    }

    auto md::block() -> utf8_string
    {
        if (is_eof()) { return {}; }
        utf8_string const& line {Lines[CurrentPos]};
        if (line.empty() || LeadingSpaces(line) == line.size()) {
            ++CurrentPos;
            return {};
        }
        if (IsFencedCodeOpen(line)) { return fenced_code(); }
        if (LeadingSpaces(line) >= 4) { return indented_code(); }
        if (IsThematicBreak(line)) {
            ++CurrentPos;
            return "<hr>";
        }
        if (i32 level {HeadingLevel(line)}) { return atx_heading(level); }
        if (IsBlockquoteLine(line)) { return blockquote(); }
        if (IsBulletListItem(line)) { return list(false); }
        i32 num {0};
        if (IsOrderedListItem(line, num)) { return list(true); }
        return paragraph();
    }

    auto md::fenced_code() -> utf8_string
    {
        usize const openPos {LeadingSpaces(Lines[CurrentPos])};
        char const  fenceChar {Lines[CurrentPos][openPos]};
        usize       fenceLen {0};
        while (openPos + fenceLen < Lines[CurrentPos].size() && Lines[CurrentPos][openPos + fenceLen] == fenceChar) { ++fenceLen; }

        utf8_string lang {Lines[CurrentPos].substr(openPos + fenceLen)};
        while (!lang.empty() && lang.back() == ' ') { lang.pop_back(); }
        ++CurrentPos;

        utf8_string out {lang.empty()
                             ? utf8_string {"<pre><code>"}
                             : std::format(R"(<pre><code class="language-{}">)", lang)};

        while (!is_eof()) {
            utf8_string const& line {Lines[CurrentPos]};
            usize const        linePos {LeadingSpaces(line)};
            usize              runLen {0};
            while (linePos + runLen < line.size() && line[linePos + runLen] == fenceChar) { ++runLen; }
            bool const isClosingFence {
                runLen >= fenceLen
                && std::ranges::all_of(line.begin() + static_cast<isize>(linePos + runLen),
                                       line.end(), [](char x) { return x == ' '; })};
            if (isClosingFence) {
                ++CurrentPos;
                break;
            }
            EscapeRange(out, line, 0, line.size());
            out += '\n';
            ++CurrentPos;
        }
        return out + "</code></pre>";
    }

    auto md::indented_code() -> utf8_string
    {
        utf8_string out {"<pre><code>"};
        while (!is_eof()) {
            utf8_string const& line {Lines[CurrentPos]};
            if (line.empty()) {
                out += '\n';
                ++CurrentPos;
                continue;
            }
            if (LeadingSpaces(line) < 4) { break; }
            EscapeRange(out, line, 4, line.size());
            out += '\n';
            ++CurrentPos;
        }
        return out + "</code></pre>";
    }

    auto md::atx_heading(i32 level) -> utf8_string
    {
        utf8_string const& line {Lines[CurrentPos++]};
        usize const        start {LeadingSpaces(line) + static_cast<usize>(level) + 1};
        usize              end {line.size()};
        while (end > start && line[end - 1] == '#') { --end; }
        while (end > start && line[end - 1] == ' ') { --end; }
        return std::format("<h{0}>{1}</h{0}>", level, ParseInline(line, start, end));
    }

    auto md::blockquote() -> utf8_string
    {
        std::vector<utf8_string> inner;
        while (!is_eof() && IsBlockquoteLine(Lines[CurrentPos])) {
            usize contentStart {LeadingSpaces(Lines[CurrentPos]) + 1};
            if (contentStart < Lines[CurrentPos].size() && Lines[CurrentPos][contentStart] == ' ') { ++contentStart; }
            inner.push_back(Lines[CurrentPos].substr(contentStart));
            ++CurrentPos;
        }
        md sub;
        sub.Lines = inner;
        return std::format("<blockquote>{}</blockquote>", sub.parse());
    }

    auto md::list(bool ordered) -> utf8_string
    {
        i32 startNum {1};
        if (ordered) { IsOrderedListItem(Lines[CurrentPos], startNum); }
        utf8_string out {ordered
                             ? (startNum != 1 ? std::format(R"(<ol start="{}">)", startNum) : utf8_string {"<ol>"})
                             : utf8_string {"<ul>"}};

        while (!is_eof()) {
            utf8_string const& line {Lines[CurrentPos]};
            if (line.empty()) { break; }
            i32  num {0};
            bool isItem {ordered ? IsOrderedListItem(line, num) : IsBulletListItem(line)};
            if (!isItem) { break; }

            usize const itemIndent {LeadingSpaces(line)};
            usize       textStart {itemIndent};
            if (ordered) {
                while (textStart < line.size() && line[textStart] != '.' && line[textStart] != ')') { ++textStart; }
                textStart += 2;
            } else {
                textStart += 2;
            }
            utf8_string const itemText {line.substr(textStart)};
            ++CurrentPos;

            std::vector<utf8_string> continuation;
            while (!is_eof()) {
                utf8_string const& next {Lines[CurrentPos]};
                if (next.empty() || LeadingSpaces(next) <= itemIndent) { break; }
                usize const stripLen {itemIndent + 2 <= next.size() ? itemIndent + 2 : 0};
                continuation.push_back(next.substr(stripLen));
                ++CurrentPos;
            }

            if (!continuation.empty()) {
                md sub;
                sub.Lines = continuation;
                out += std::format("<li>{}{}</li>", ParseInline(itemText, 0, itemText.size()), sub.parse());
            } else {
                out += std::format("<li>{}</li>", ParseInline(itemText, 0, itemText.size()));
            }
        }
        return std::format("{}{}", out, ordered ? "</ol>" : "</ul>");
    }

    auto md::paragraph() -> utf8_string
    {
        utf8_string text;
        while (!is_eof()) {
            utf8_string const& line {Lines[CurrentPos]};
            if (line.empty() || LeadingSpaces(line) == line.size()) { break; }
            if (IsFencedCodeOpen(line) || IsThematicBreak(line) || HeadingLevel(line) || IsBlockquoteLine(line)) { break; }
            i32 num {0};
            if (IsBulletListItem(line) || IsOrderedListItem(line, num)) { break; }
            // peek ahead: stop before consuming a setext underline
            utf8_string const& nextLine {(CurrentPos + 1 < Lines.size()) ? Lines[CurrentPos + 1] : utf8_string {}};
            if (AllSameChar(nextLine, '=') || AllSameChar(nextLine, '-')) {
                if (!text.empty()) { text += ' '; }
                text += line;
                ++CurrentPos;
                break;
            }
            if (!text.empty()) { text += ' '; }
            text += line;
            ++CurrentPos;
        }

        // setext heading check
        if (!is_eof()) {
            utf8_string const& underline {Lines[CurrentPos]};
            if (AllSameChar(underline, '=')) {
                ++CurrentPos;
                return std::format("<h1>{}</h1>", ParseInline(text, 0, text.size()));
            }
            if (AllSameChar(underline, '-')) {
                ++CurrentPos;
                return std::format("<h2>{}</h2>", ParseInline(text, 0, text.size()));
            }
        }
        return std::format("<p>{}</p>", ParseInline(text, 0, text.size()));
    }

}

auto md_to_html(utf8_string_view md) -> utf8_string
{
    detail::md   p;
    io::isstream ss {std::as_bytes(std::span {md})};

    while (!ss.is_eof()) {
        utf8_string line {ss.read_string_until('\n')};
        if (!line.empty() && line.back() == '\r') { line.pop_back(); }
        p.Lines.push_back(line);
    }

    return p.parse();
}

}
