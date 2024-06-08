// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/CSV.hpp"

#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::data::csv {

static byte const LineEnding {'\n'};

auto table::load(path const& file, settings s) noexcept -> load_status
{
    if (auto fs {io::ifstream::Open(file)}) {
        return parse(io::read_as_string(file), s) ? load_status::Ok : load_status::Error;
    }

    return load_status::FileNotFound;
}

auto table::load(istream& in, settings s) noexcept -> load_status
{
    return parse(in.read_string(in.size_in_bytes()), s) ? load_status::Ok : load_status::Error;
}

auto table::parse(string const& csv, settings s) -> bool
{
    Header.clear();
    Rows.clear();

    bool isInQuote {false};
    bool isInHeader {s.HasHeader};

    string              value;
    std::vector<string> row;

    for (auto const& c : csv) {
        if (isInQuote) {
            if (c == s.Quote) {
                if (value.empty()) {
                    value += c;
                }
                isInQuote = false;
            } else {
                value += c;
            }
        } else {
            if (c == s.Quote) {
                isInQuote = true;
            } else if (c == s.Separator) {
                row.push_back(value);
                value.clear();
            } else if (c == ' ' || c == '\t' || c == '\r') {
                continue;
            } else if (c == LineEnding) {
                row.push_back(value);
                if (isInHeader) {
                    Header     = row;
                    isInHeader = false;
                } else {
                    Rows.push_back(row);
                }
                value.clear();
                row.clear();
            } else {
                value += c;
            }
        }
    }

    if (!value.empty()) {
        row.push_back(value);
    }
    if (!row.empty()) {
        if (isInHeader) {
            Header = row;
        } else {
            Rows.push_back(row);
        }
    }

    return true;
}

void static write_line(std::vector<string> const& row, ostream& ss, settings const& s)
{
    for (usize i {0}; i < row.size(); ++i) {
        string const& value {row[i]};
        if (value.find(s.Separator) != string::npos || value.find(' ') != string::npos || value.find('\t') != string::npos) {
            ss << s.Quote << value << s.Quote;
        } else {
            ss << value;
        }
        if (i != row.size() - 1) {
            ss << s.Separator;
        }
    }
    ss << LineEnding;
}

auto table::save(path const& file, settings s) const -> bool
{
    io::ofstream stream {file};
    return save(stream, s);
}

auto table::save(ostream& out, settings s) const -> bool
{
    if (!Header.empty()) {
        write_line(Header, out, s);
    }

    for (auto const& row : Rows) {
        write_line(row, out, s);
    }

    return true;
}

}
