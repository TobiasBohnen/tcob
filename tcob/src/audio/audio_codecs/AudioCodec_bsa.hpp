// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/audio/AudioSource.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::audio::detail {
////////////////////////////////////////////////////////////

class bsa_decoder final : public decoder {
public:
    void seek_from_start(milliseconds pos) override;

protected:
    auto open() -> std::optional<buffer::info> override;
    auto decode(std::span<f32> outputSamples) -> i32 override;

private:
    buffer::info _info {};
};

////////////////////////////////////////////////////////////

class bsa_encoder final : public encoder {
public:
    auto encode(std::span<f32 const> samples, buffer::info const& info, ostream& out) const -> bool override;
};

}
