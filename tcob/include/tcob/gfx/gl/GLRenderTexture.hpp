// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>

namespace tcob::gl {
class RenderTexture final : public RenderTarget {
public:
    void create(const SizeU& size);

    auto size() const -> SizeU override;
};
}