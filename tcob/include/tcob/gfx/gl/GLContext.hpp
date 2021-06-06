// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Size.hpp>

struct SDL_Window;

namespace tcob::gl {
class Context final {
public:
    Context(const PointU& loc, const SizeU& size, u32 aa = 0);
    ~Context();

    auto window_handle() const -> SDL_Window*;

private:
    void* _context { nullptr };
    SDL_Window* _window { nullptr };
};
}