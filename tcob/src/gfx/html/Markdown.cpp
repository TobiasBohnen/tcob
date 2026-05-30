// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/html/Markdown.hpp"

#include <algorithm>
#include <format>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/StringUtils.hpp"

namespace tcob::gfx::html {

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

static void EscapeRange(utf8_string& out, utf8_string_view s, usize const from, usize const to)
{
    for (usize i {from}; i < to; ++i) { EscapeChar(out, s[i]); }
}

static auto IsSpace(char const c) -> bool { return c == ' ' || c == '\t'; }

static auto ParseInline(utf8_string_view s, usize const from, usize const to) -> utf8_string
{
    utf8_string retValue;
    for (usize i {from}; i < to;) {
        char const c {s[i]};

        // inline html
        if (c == '<' && i + 1 < to) {
            char const next {s[i + 1]};
            if (helper::is_ascii_alpha(next) || next == '/' || next == '!' || next == '?') {
                usize const closePos {s.find('>', i + 1)};
                if (closePos != utf8_string_view::npos && closePos < to) {
                    retValue += s.substr(i, closePos - i + 1);
                    i = closePos + 1;
                    continue;
                }
            }
        }

        // backslash escape
        if (c == '\\' && i + 1 < to) {
            EscapeChar(retValue, s[i + 1]);
            i += 2;
            continue;
        }

        // inline code  `...`  or  ``...``
        if (c == '`') {
            usize runLen {0};
            while (i + runLen < to && s[i + runLen] == '`') { ++runLen; }

            utf8_string_view openFence {s.substr(i, runLen)};
            usize const      closePos {s.find(openFence, i + runLen)};
            if (closePos != utf8_string_view::npos && closePos + runLen <= to) {
                utf8_string escaped;
                EscapeRange(escaped, s, i + runLen, closePos);
                retValue += std::format("<code>{}</code>", escaped);
                i = closePos + runLen;
                continue;
            }
            EscapeChar(retValue, c);
            ++i;
            continue;
        }

        // bold  **...**  or  __...__
        if ((c == '*' || c == '_') && i + 1 < to && s[i + 1] == c) {
            if (c == '_' && i > from && !IsSpace(s[i - 1])) {
                EscapeChar(retValue, c);
                ++i;
                continue;
            }
            usize closePos {utf8_string_view::npos};
            for (usize j {i + 2}; j + 1 < to; ++j) {
                if (s[j] == c && s[j + 1] == c) {
                    usize runEnd {j + 2};
                    while (runEnd < to && s[runEnd] == c) { ++runEnd; }
                    usize const runLen {runEnd - j};

                    if (c == '_' && runEnd < to && !IsSpace(s[runEnd])) {
                        j = runEnd - 1;
                        continue;
                    }
                    closePos = j + (runLen - 2);
                    break;
                }
            }
            if (closePos != utf8_string_view::npos && closePos + 2 <= to) {
                retValue += std::format("<strong>{}</strong>", ParseInline(s, i + 2, closePos));
                i = closePos + 2;
                continue;
            }
        }

        // italic  *...*  or  _..._
        if (c == '*' || c == '_') {
            if (c == '_' && i > from && !IsSpace(s[i - 1])) {
                EscapeChar(retValue, c);
                ++i;
                continue;
            }
            usize closePos {utf8_string_view::npos};
            for (usize j {i + 1}; j < to; ++j) {
                if (s[j] == c) {
                    bool const prevSame {j > from && s[j - 1] == c};
                    bool const nextSame {j + 1 < to && s[j + 1] == c};
                    if (prevSame || nextSame) { continue; }
                    if (c == '_' && j + 1 < to && !IsSpace(s[j + 1])) { continue; }
                    closePos = j;
                    break;
                }
            }
            if (closePos != utf8_string_view::npos && closePos < to) {
                retValue += std::format("<em>{}</em>", ParseInline(s, i + 1, closePos));
                i = closePos + 1;
                continue;
            }
            EscapeChar(retValue, c);
            ++i;
            continue;
        }

        // strikethrough  ~...~  or  ~~...~~
        if (c == '~') {
            usize runLen {0};
            while (i + runLen < to && s[i + runLen] == '~') { ++runLen; }

            if (runLen == 1 || runLen == 2) {
                utf8_string_view closeRun {s.substr(i, runLen)};
                usize const      closePos {s.find(closeRun, i + runLen)};

                if (closePos != utf8_string_view::npos && closePos + runLen <= to) {
                    retValue += std::format("<del>{}</del>", ParseInline(s, i + runLen, closePos));
                    i = closePos + runLen;
                    continue;
                }
            }
        }

        // inline color {color}(text)
        if (c == '{') {
            usize const closeBrace {s.find('}', i + 1)};
            if (closeBrace != utf8_string_view::npos && closeBrace + 1 < to && s[closeBrace + 1] == '(') {
                usize const closeParen {s.find(')', closeBrace + 2)};
                if (closeParen != utf8_string_view::npos && closeParen < to) {
                    utf8_string_view col {s.substr(i + 1, closeBrace - i - 1)};
                    if (color::FromString(col) == colors::Transparent) {
                        retValue += s.substr(i, closeParen - i + 1);
                        i = closeParen + 1;
                        continue;
                    }

                    utf8_string const text {ParseInline(s, closeBrace + 2, closeParen)};
                    retValue += std::format(R"(<span style="color:{}">{}</span>)", col, text);
                    i = closeParen + 1;
                    continue;
                }
            }
        }

        // image  ![alt](url)
        if (c == '!' && i + 1 < to && s[i + 1] == '[') {
            usize const altEnd {s.find(']', i + 2)};
            if (altEnd != utf8_string_view::npos && altEnd + 1 < to && s[altEnd + 1] == '(') {
                usize const urlEnd {s.find(')', altEnd + 2)};
                if (urlEnd != utf8_string_view::npos && urlEnd < to) {
                    utf8_string url;
                    utf8_string alt;
                    EscapeRange(url, s, altEnd + 2, urlEnd);
                    EscapeRange(alt, s, i + 2, altEnd);
                    retValue += std::format(R"(<img src="{}" alt="{}">)", url, alt);
                    i = urlEnd + 1;
                    continue;
                }
            }
        }

        // link  [text](url)
        if (c == '[') {
            usize const textEnd {s.find(']', i + 1)};
            if (textEnd != utf8_string_view::npos && textEnd + 1 < to && s[textEnd + 1] == '(') {
                usize const urlEnd {s.find(')', textEnd + 2)};
                if (urlEnd != utf8_string_view::npos && urlEnd < to) {
                    utf8_string url;
                    EscapeRange(url, s, textEnd + 2, urlEnd);
                    retValue += std::format(R"(<a href="{}">{}</a>)", url, ParseInline(s, i + 1, textEnd));
                    i = urlEnd + 1;
                    continue;
                }
            }
        }

        EscapeChar(retValue, c);
        ++i;
    }
    return retValue;
}

static auto ParseInline(utf8_string_view s) -> utf8_string
{
    return ParseInline(s, 0, s.size());
}

static auto LeadingSpaces(utf8_string_view s) -> usize
{
    usize n {0};
    while (n < s.size() && s[n] == ' ') { ++n; }
    return n;
}

static auto FirstChar(utf8_string_view s) -> char
{
    usize const pos {LeadingSpaces(s)};
    return pos < s.size() ? s[pos] : 0;
}

static auto HeadingLevel(utf8_string_view s) -> i32
{
    usize const start {LeadingSpaces(s)};
    usize       hashes {0};
    while (start + hashes < s.size() && s[start + hashes] == '#') { ++hashes; }
    if (!hashes || hashes > 6 || start + hashes >= s.size() || s[start + hashes] != ' ') { return 0; }
    return static_cast<i32>(hashes);
}

static auto IsThematicBreak(utf8_string_view s) -> bool
{
    char const c {FirstChar(s)};
    if (c != '-' && c != '*' && c != '_') { return false; }
    i32 count {0};
    for (char const x : s) {
        if (x == c) {
            ++count;
        } else if (x != ' ') {
            return false;
        }
    }
    return count >= 3;
}

static auto IsFencedCodeOpen(utf8_string_view s) -> bool
{
    usize const pos {LeadingSpaces(s)};
    return pos + 2 < s.size()
        && ((s[pos] == '`' && s[pos + 1] == '`' && s[pos + 2] == '`')
            || (s[pos] == '~' && s[pos + 1] == '~' && s[pos + 2] == '~'));
}

static auto IsBulletListItem(utf8_string_view s) -> bool
{
    usize const pos {LeadingSpaces(s)};
    char const  c {FirstChar(s)};
    return (c == '-' || c == '*' || c == '+') && pos + 1 < s.size() && s[pos + 1] == ' ';
}

static auto IsOrderedListItem(utf8_string_view s, i32& startNum) -> bool
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

static auto IsBlockquoteLine(utf8_string_view s) -> bool { return FirstChar(s) == '>'; }

static auto IsTable(utf8_string_view current, utf8_string_view next) -> bool
{
    current = helper::trim(current);
    next    = helper::trim(next);

    auto const hasOuterPipes {[](utf8_string_view s) {
        return s.size() >= 2
            && s.front() == '|'
            && s.back() == '|';
    }};

    return hasOuterPipes(current) && hasOuterPipes(next) && next.contains('-');
}

static auto AllSameChar(utf8_string_view s, char const c) -> bool
{
    return !s.empty() && std::ranges::all_of(s, [c](char const x) { return x == c; });
}

struct md_parser {
    std::vector<utf8_string_view> Lines {};
    usize                         CurrentPos {0};

    auto is_eof() const -> bool { return CurrentPos >= Lines.size(); }

    auto parse() -> utf8_string
    {
        utf8_string retValue;
        while (!is_eof()) { retValue += block(); }
        return retValue;
    }

    auto block() -> utf8_string
    {
        if (is_eof()) { return {}; }
        utf8_string_view line {Lines[CurrentPos]};
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
        if (i32 const level {HeadingLevel(line)}) { return atx_heading(level); }
        if (IsBlockquoteLine(line)) { return blockquote(); }
        if (IsBulletListItem(line)) { return list(false); }
        i32 num {0};
        if (IsOrderedListItem(line, num)) { return list(true); }
        if (CurrentPos + 1 < Lines.size() && IsTable(line, Lines[CurrentPos + 1])) {
            return table();
        }
        return paragraph();
    }

    auto fenced_code() -> utf8_string
    {
        utf8_string_view openLine {Lines[CurrentPos]};
        usize const      openPos {LeadingSpaces(openLine)};
        char const       fenceChar {openLine[openPos]};
        usize            fenceLen {0};
        while (openPos + fenceLen < openLine.size() && openLine[openPos + fenceLen] == fenceChar) { ++fenceLen; }

        utf8_string_view lang {openLine.substr(openPos + fenceLen)};
        while (!lang.empty() && lang.back() == ' ') { lang.remove_suffix(1); }
        ++CurrentPos;

        utf8_string retValue {lang.empty()
                                  ? utf8_string {"<pre><code>"}
                                  : std::format(R"(<pre><code class="language-{}">)", lang)};

        while (!is_eof()) {
            utf8_string_view line {Lines[CurrentPos]};
            usize const      linePos {LeadingSpaces(line)};
            usize            runLen {0};
            while (linePos + runLen < line.size() && line[linePos + runLen] == fenceChar) { ++runLen; }

            bool const isClosingFence {
                runLen >= fenceLen
                && std::ranges::all_of(line.substr(linePos + runLen), [](char const x) { return x == ' '; })};
            if (isClosingFence) {
                ++CurrentPos;
                break;
            }
            EscapeRange(retValue, line, 0, line.size());
            retValue += '\n';
            ++CurrentPos;
        }
        return retValue + "</code></pre>";
    }

    auto indented_code() -> utf8_string
    {
        utf8_string retValue {"<pre><code>"};
        while (!is_eof()) {
            utf8_string_view line {Lines[CurrentPos]};
            if (line.empty()) {
                retValue += '\n';
                ++CurrentPos;
                continue;
            }
            if (LeadingSpaces(line) < 4) { break; }
            EscapeRange(retValue, line, 4, line.size());
            retValue += '\n';
            ++CurrentPos;
        }
        return retValue + "</code></pre>";
    }

    auto atx_heading(i32 const level) -> utf8_string
    {
        utf8_string_view line {Lines[CurrentPos++]};
        usize const      start {LeadingSpaces(line) + static_cast<usize>(level) + 1};
        usize            end {line.size()};
        while (end > start && line[end - 1] == '#') { --end; }
        while (end > start && line[end - 1] == ' ') { --end; }
        return std::format("<h{0}>{1}</h{0}>", level, ParseInline(line, start, end));
    }

    auto blockquote() -> utf8_string
    {
        std::vector<utf8_string_view> inner {};
        while (!is_eof() && IsBlockquoteLine(Lines[CurrentPos])) {
            utf8_string_view line {Lines[CurrentPos]};
            usize            contentStart {LeadingSpaces(line) + 1};
            if (contentStart < line.size() && line[contentStart] == ' ') { ++contentStart; }
            inner.push_back(line.substr(contentStart));
            ++CurrentPos;
        }
        md_parser sub {};
        sub.Lines = inner;
        return std::format("<blockquote>{}</blockquote>", sub.parse());
    }

    auto list(bool const ordered) -> utf8_string
    {
        i32 startNum {1};
        if (ordered) { IsOrderedListItem(Lines[CurrentPos], startNum); }
        utf8_string retValue {ordered
                                  ? (startNum != 1 ? std::format(R"(<ol start="{}">)", startNum) : utf8_string {"<ol>"})
                                  : utf8_string {"<ul>"}};

        while (!is_eof()) {
            utf8_string_view line {Lines[CurrentPos]};
            if (line.empty()) { break; }
            i32        num {0};
            bool const isItem {ordered ? IsOrderedListItem(line, num) : IsBulletListItem(line)};
            if (!isItem) { break; }

            usize const itemIndent {LeadingSpaces(line)};
            usize       textStart {itemIndent};
            if (ordered) {
                while (textStart < line.size() && line[textStart] != '.' && line[textStart] != ')') { ++textStart; }
                textStart += 2;
            } else {
                textStart += 2;
            }
            utf8_string_view itemText {line.substr(textStart)};
            ++CurrentPos;

            std::vector<utf8_string_view> continuation {};
            while (!is_eof()) {
                utf8_string_view next {Lines[CurrentPos]};
                if (next.empty() || LeadingSpaces(next) <= itemIndent) { break; }
                usize const stripLen {textStart <= next.size() ? textStart : 0};
                continuation.push_back(next.substr(stripLen));
                ++CurrentPos;
            }

            if (!continuation.empty()) {
                md_parser sub {};
                sub.Lines = continuation;
                retValue += std::format("<li>{}{}</li>", ParseInline(itemText), sub.parse());
            } else {
                retValue += std::format("<li>{}</li>", ParseInline(itemText));
            }
        }
        return std::format("{}{}", retValue, ordered ? "</ol>" : "</ul>");
    }

    auto table() -> utf8_string
    {
        std::vector<utf8_string_view> rows {};
        while (!is_eof() && Lines[CurrentPos].contains('|')) {
            rows.push_back(Lines[CurrentPos++]);
        }

        if (rows.size() < 2) { return {}; }

        auto const parseRow {[](utf8_string_view row) {
            usize const start {row.find('|') + 1};
            usize const end {row.rfind('|')};
            if (start >= end) { return std::vector<utf8_string_view> {}; }

            utf8_string_view center {row.substr(start, end - start)};
            auto const       rawCells {helper::split(center, '|')};

            std::vector<utf8_string_view> cleanCells {};
            cleanCells.reserve(rawCells.size());
            for (auto const& cell : rawCells) {
                cleanCells.push_back(helper::trim(cell));
            }
            return cleanCells;
        }};

        auto const header_cells {parseRow(rows[0])};
        if (header_cells.empty()) { return {}; }

        utf8_string retValue {"<table><thead><tr>"};
        for (auto const& h : header_cells) {
            retValue += std::format("<th>{}</th>", ParseInline(h));
        }
        retValue += "</tr></thead><tbody>";

        for (usize i {2}; i < rows.size(); ++i) {
            auto const cells {parseRow(rows[i])};
            if (cells.empty()) { continue; }

            retValue += "<tr>";
            for (usize c {0}; c < header_cells.size(); ++c) {
                retValue += "<td>";
                if (c < cells.size()) {
                    retValue += ParseInline(cells[c]);
                }
                retValue += "</td>";
            }
            retValue += "</tr>";
        }

        return retValue + "</tbody></table>";
    }

    auto paragraph() -> utf8_string
    {
        utf8_string text;
        while (!is_eof()) {
            utf8_string_view line {Lines[CurrentPos]};
            if (line.empty() || LeadingSpaces(line) == line.size()) { break; }
            if (IsFencedCodeOpen(line) || IsThematicBreak(line) || HeadingLevel(line) || IsBlockquoteLine(line) || IsBulletListItem(line)) { break; }
            i32 num {0};
            if (IsOrderedListItem(line, num)) { break; }
            if (CurrentPos + 1 < Lines.size() && IsTable(line, Lines[CurrentPos + 1])) {
                break;
            }

            utf8_string_view nextLine {(CurrentPos + 1 < Lines.size()) ? Lines[CurrentPos + 1] : utf8_string_view {}};
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

        if (!is_eof()) {
            utf8_string_view underline {Lines[CurrentPos]};
            if (AllSameChar(underline, '=')) {
                ++CurrentPos;
                return std::format("<h1>{}</h1>", ParseInline(text));
            }
            if (AllSameChar(underline, '-')) {
                ++CurrentPos;
                return std::format("<h2>{}</h2>", ParseInline(text));
            }
        }
        return std::format("<p>{}</p>", ParseInline(text));
    }
};

auto md_to_html(utf8_string_view md) -> utf8_string
{
    md_parser p {};

    usize pos {0};
    while (pos < md.size()) {
        usize next {md.find('\n', pos)};
        if (next == utf8_string_view::npos) {
            next = md.size();
        }

        utf8_string_view line {md.substr(pos, next - pos)};
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }

        p.Lines.push_back(line);
        pos = next + 1;
    }

    return p.parse();
}

}
