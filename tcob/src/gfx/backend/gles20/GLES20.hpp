// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Logger.hpp" // IWYU pragma: keep
#include "tcob/gfx/Gfx.hpp"     // IWYU pragma: keep

namespace tcob::gfx::gles20 {

#if defined(TCOB_DEBUG)

    #define GLCHECK(_CALL) /*NOLINT*/                                                                \
        do {                                                                                         \
            _CALL;                                                                                   \
            const GLenum gl_err {glGetError()};                                                      \
            if (gl_err != 0)                                                                         \
                logger::Error("GLES: error " + std::to_string(gl_err) + " returned from " + #_CALL); \
                                                                                                     \
        } while (0)
#else
    #define GLCHECK(_CALL) _CALL
#endif

}
