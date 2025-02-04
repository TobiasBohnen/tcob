// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Camera.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct render_properties {
    mat4    ViewMatrix {};
    rect_i  Viewport {rect_i::Zero};
    point_i MousePosition {point_i::Zero};
    f32     Time {0.0f};
    bool    Debug {false};

    bool UseDefaultFramebuffer {false};
};

////////////////////////////////////////////////////////////

class TCOB_API render_target {
public:
    explicit render_target(texture* tex);
    virtual ~render_target();

    prop_fn<size_i> Size;

    color                 ClearColor {colors::DarkGray};
    std::optional<rect_i> ScissorRect;

    auto camera() -> gfx::camera&;

    void clear() const;
    void clear(color c) const;

    auto copy_to_image() const -> image;

    // Renderer
    void virtual prepare_render(bool debug = false);
    void virtual finalize_render() const;
    void bind_material(material const* mat) const;
    void unbind_material() const;

    template <std::derived_from<render_backend::render_target_base> T>
    auto get_impl() const -> T*;

protected:
    void virtual on_clear(color c) const;
    auto virtual get_size() const -> size_i = 0;
    void virtual set_size(size_i size);

private:
    std::unique_ptr<render_backend::render_target_base> _impl;
    gfx::camera                                         _camera;
};

////////////////////////////////////////////////////////////

class default_render_target : public render_target {
public:
    default_render_target(window* win);

protected:
    auto get_size() const -> size_i override;

    void prepare_render(bool) override;

private:
    void set_size(size_i newsize) override;

    window* _window;
};

}

#include "RenderTarget.inl"
