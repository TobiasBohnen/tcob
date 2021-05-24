// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once
#include <tcob/tcob_config.hpp>

struct SDL_Window;

namespace tcob::gl {
class Context final {
public:
    Context(SDL_Window* window);
    ~Context();

private:
    void* _context { nullptr };
};
}