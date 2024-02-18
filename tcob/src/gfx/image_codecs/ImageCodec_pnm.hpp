// Copyright (c) 2023 Tobias Bohnen
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

namespace pnm {
    constexpr i32 BPP {3};

    enum class format {
        P1 = 1,
        P2 = 2,
        P3 = 3,
        P4 = 4,
        P5 = 5,
        P6 = 6
    };

    struct header {
        format Format {};
        string FormatString;
        bool   IsAscii {};
        i16    MaxValue {};
        u32    Width {};
        u32    Height {};

        void read(istream& reader);
    };
}

////////////////////////////////////////////////////////////

class pnm_decoder final : public image_decoder {
public:
    auto decode(istream& in) -> std::optional<image> override;
    auto decode_header(istream& in) -> std::optional<image::info> override;

private:
    pnm::header _header;
};

}
