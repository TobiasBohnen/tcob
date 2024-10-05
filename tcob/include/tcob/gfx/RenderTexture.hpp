// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API render_texture : public texture, public render_target {
public:
    render_texture();

    auto static GetTexcoords() -> rect_f;

protected:
    auto get_size() const -> size_i override;
};

}
