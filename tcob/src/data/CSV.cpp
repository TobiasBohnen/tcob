// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/CSV.hpp"

#include <span>
#include <utility>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::data {

static char const LineEnding {'\n'};

auto csv_table::load(path const& file, settings s) noexcept -> bool
{
    io::ifstream fs {file};
    return load(fs, s);
}

auto csv_table::load(io::istream& in, settings s) noexcept -> bool
{
    if (!in) { return false; }
    return parse(in.read_string(in.size_in_bytes()), s);
}

auto csv_table::parse(string const& csv, settings s) -> bool
{
    Header.clear();
    Rows.clear();

    bool isInQuote {false};
    bool isInHeader {s.HasHeader};

    string value;
    value.reserve(64);

    std::vector<string> row;
    row.reserve(16);

    auto const finalize {[&](std::vector<string>& r) {
        if (isInHeader) {
            Header     = std::move(r);
            isInHeader = false;
        } else {
            if (Rows.size().Height == 0) {
                Rows = grid<string> {size_i {static_cast<i32>(r.size()), 0}};
            } else if (Rows.width() != static_cast<i32>(r.size())) {
                return false;
            }
            Rows.append(r);
        }
        r.clear();
        return true;
    }};

    for (char c : csv) {
        if (isInQuote) {
            if (c == s.Quote) {
                isInQuote = false;
            } else {
                value += c;
            }
            continue;
        }

        if (c == s.Quote) {
            isInQuote = true;
        } else if (c == s.Separator) {
            row.push_back(std::move(value));
            value.clear();
        } else if (c == LineEnding) {
            row.push_back(std::move(value));
            value.clear();
            if (!finalize(row)) { return false; }
        } else if (c == ' ' || c == '\t' || c == '\r') {
            continue;
        } else {
            value += c;
        }
    }

    if (!value.empty()) { row.push_back(std::move(value)); }
    if (!row.empty()) {
        if (!finalize(row)) { return false; }
    }

    if (!Header.empty() && Rows.width() != static_cast<i32>(Header.size())) {
        return false;
    }
    return true;
}

static void write_line(std::span<string const> row, io::ostream& ss, csv_table::settings const& s)
{
    for (usize i {0}; i < row.size(); ++i) {
        string const& value {row[i]};
        if (value.find(s.Separator) != string::npos || value.find(' ') != string::npos || value.find('\t') != string::npos) {
            ss << s.Quote << value << s.Quote;
        } else {
            ss << value;
        }
        if (i != row.size() - 1) { ss << s.Separator; }
    }
    ss << LineEnding;
}

auto csv_table::save(path const& file, settings s) const -> bool
{
    io::ofstream stream {file};
    return save(stream, s);
}

auto csv_table::save(io::ostream& out, settings s) const -> bool
{
    if (!Header.empty()) { write_line(Header, out, s); }
    for (isize j {0}; j < Rows.size().Height; ++j) {
        write_line(Rows.row(j), out, s);
    }

    return true;
}

}
