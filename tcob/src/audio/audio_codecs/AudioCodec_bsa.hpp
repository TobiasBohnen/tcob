// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <span>

#include "tcob/audio/Buffer.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class bsa_decoder final : public decoder {
public:
    void seek_from_start(milliseconds pos) override;

protected:
    auto open() -> std::optional<buffer::information> override;
    auto decode(std::span<f32> outputSamples) -> i32 override;

private:
    buffer::information _info {};
};

////////////////////////////////////////////////////////////

class bsa_encoder final : public encoder {
public:
    auto encode(std::span<f32 const> samples, buffer::information const& info, io::ostream& out) const -> bool override;
};

}
