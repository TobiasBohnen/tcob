// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API zlib_filter {
public:
    explicit zlib_filter(i32 complevel = -1);

    auto to(std::span<std::byte const> bytes) const -> std::vector<std::byte>;
    auto from(std::span<std::byte const> bytes) const -> std::vector<std::byte>;

private:
    static constexpr usize BUFFER_SIZE {8192};

    i32 _level;
};

////////////////////////////////////////////////////////////

class TCOB_API base64_filter {
public:
    auto to(std::span<std::byte const> bytes) const -> std::vector<std::byte>;
    auto from(std::span<std::byte const> bytes) const -> std::vector<std::byte>;
};

////////////////////////////////////////////////////////////

class TCOB_API z85_filter {
public:
    auto to(std::span<std::byte const> bytes) const -> std::vector<std::byte>;
    auto from(std::span<std::byte const> bytes) const -> std::vector<std::byte>;
};

////////////////////////////////////////////////////////////

class TCOB_API reverser_filter {
public:
    auto to(std::span<std::byte const> bytes) const -> std::vector<std::byte>;
    auto from(std::span<std::byte const> bytes) const -> std::vector<std::byte>;
};

}
