// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

class qoi_decoder final : public image_decoder {
public:
    auto decode(io::istream& in) -> std::optional<image> override;
    auto decode_info(io::istream& in) -> std::optional<image::information> override;
};

////////////////////////////////////////////////////////////

class qoi_encoder final : public image_encoder {
public:
    auto encode(image const& image, io::ostream& out) const -> bool override;
};

}
