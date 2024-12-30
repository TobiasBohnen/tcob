// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Sound.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API speech_generator final {
public:
    speech_generator() = default;

    auto create_buffer [[nodiscard]] (std::string const& text) -> buffer;
    auto create_sound [[nodiscard]] (std::string const& text) -> std::shared_ptr<sound>;

private:
};
}
