// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <vector>

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API filter_base {
public:
    filter_base()                                                     = default;
    filter_base(filter_base const& other) noexcept                    = delete;
    auto operator=(filter_base const& other) noexcept -> filter_base& = delete;
    virtual ~filter_base()                                            = default;

    auto virtual to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>>   = 0;
    auto virtual from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API zlib_filter : public filter_base {
public:
    explicit zlib_filter(i32 complevel = -1);

    auto to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;
    auto from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;

private:
    i32 _level;
};

////////////////////////////////////////////////////////////

class TCOB_API base64_filter : public filter_base {
public:
    auto to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;
    auto from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;
};

////////////////////////////////////////////////////////////

class TCOB_API z85_filter : public filter_base {
public:
    auto to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;
    auto from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;
};

////////////////////////////////////////////////////////////

class TCOB_API reverser_filter : public filter_base {
public:
    auto to(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;
    auto from(std::span<ubyte const> bytes) const -> std::optional<std::vector<ubyte>> override;
};

}
