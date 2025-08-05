// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

struct SDL_Window;

namespace tcob::gfx::gles30 {
////////////////////////////////////////////////////////////

class gl_context;

class gl_window final : public tcob::gfx::render_backend::window_base {
public:
    explicit gl_window(size_i size);
    ~gl_window() override;

    auto get_vsync() const -> bool override;
    void set_vsync(bool value) override;

    void swap_buffer() const override;

    void clear(color c) const override;

    void set_viewport(rect_i const& rect) override;

    auto get_handle() const -> void* override
    {
        return _window;
    }

private:
    SDL_Window* _window {nullptr};

    std::unique_ptr<gl_context> _context;
};
}
