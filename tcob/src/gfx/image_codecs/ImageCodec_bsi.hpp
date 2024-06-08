// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace bsi {
    struct header {
        std::array<ubyte, 3> Sig {};
        size_i               Size {};
        image::format        Format {};

        void read(istream& reader);
    };
}

////////////////////////////////////////////////////////////

class bsi_decoder final : public image_decoder {
public:
    auto decode(istream& in) -> std::optional<image> override;
    auto decode_header(istream& in) -> std::optional<image::info> override;
};

////////////////////////////////////////////////////////////

class bsi_encoder final : public image_encoder {
public:
    auto encode(image const& img, ostream& out) const -> bool override;
};

}
