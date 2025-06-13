// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <vector>

namespace tcob::io {

////////////////////////////////////////////////////////////

class TCOB_API zlib_filter {
public:
    explicit zlib_filter(i32 complevel = -1);

    auto to(std::span<byte const> bytes) const -> std::vector<byte>;
    auto from(std::span<byte const> bytes) const -> std::vector<byte>;

private:
    i32 _level;
};

////////////////////////////////////////////////////////////

class TCOB_API base64_filter {
public:
    auto to(std::span<byte const> bytes) const -> std::vector<byte>;
    auto from(std::span<byte const> bytes) const -> std::vector<byte>;
};

////////////////////////////////////////////////////////////

class TCOB_API z85_filter {
public:
    auto to(std::span<byte const> bytes) const -> std::vector<byte>;
    auto from(std::span<byte const> bytes) const -> std::vector<byte>;
};

////////////////////////////////////////////////////////////

class TCOB_API reverser_filter {
public:
    auto to(std::span<byte const> bytes) const -> std::vector<byte>;
    auto from(std::span<byte const> bytes) const -> std::vector<byte>;
};

}
