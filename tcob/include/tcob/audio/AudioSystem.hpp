// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Interfaces.hpp"

struct SDL_AudioStream;

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API system final : public non_copyable {
public:
    system();
    ~system();

    static inline char const* service_name {"audio_system"};

    auto create_output(buffer::information const& info) const -> std::unique_ptr<detail::output>;

private:
    u32 _device;
};

}
