// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <optional>

#include "tcob/core/Size.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

namespace bsi {
    struct header {
        std::array<byte, 3> Sig {};
        size_i              Size {};
        image::format       Format {};

        void read(io::istream& reader);
    };
}

////////////////////////////////////////////////////////////

class bsi_decoder final : public image_decoder {
public:
    auto decode(io::istream& in) -> std::optional<image> override;
    auto decode_info(io::istream& in) -> std::optional<image::information> override;
};

////////////////////////////////////////////////////////////

class bsi_encoder final : public image_encoder {
public:
    auto encode(image const& img, io::ostream& out) const -> bool override;
};

}
