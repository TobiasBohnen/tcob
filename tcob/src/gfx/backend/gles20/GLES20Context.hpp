// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"

struct SDL_Window;

namespace tcob::gfx::gles20 {

////////////////////////////////////////////////////////////

class gl_shader;

////////////////////////////////////////////////////////////

class gl_context final : public non_copyable {
public:
    explicit gl_context(SDL_Window* window);
    ~gl_context();

    static u32 DefaultShader;
    static u32 DefaultTexturedShader;
    static u32 DefaultFontShader;

private:
    void* _context {nullptr};

    std::shared_ptr<gl_shader> _defaultShader;
    std::shared_ptr<gl_shader> _defaultTexShader;
    std::shared_ptr<gl_shader> _defaultFontShader;
};

}
