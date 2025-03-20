// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API recording final {
public:
    recording();
    ~recording();

    void start();
    auto stop() -> buffer;

private:
    std::unique_ptr<detail::audio_stream> _output;
};

}
