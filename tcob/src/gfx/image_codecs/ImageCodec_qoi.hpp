// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)

    #include <optional>

    #include <qoi/qoi.h>

    #include "tcob/core/io/Stream.hpp"
    #include "tcob/gfx/Image.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

class qoi_decoder final : public image_decoder {
public:
    auto decode(istream& in) -> std::optional<image> override;
    auto decode_header(istream& in) -> std::optional<image::info> override;
};

////////////////////////////////////////////////////////////

class qoi_encoder final : public image_encoder {
public:
    auto encode(image const& image, ostream& out) const -> bool override;
};

}

#endif
