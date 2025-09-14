// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "NullAudioSystem.hpp"

#include <cassert>
#include <memory>
#include <span>
#include <vector>

#include "tcob/audio/Audio.hpp"

namespace tcob::audio::null {

void null_audio_stream::bind() { }
void null_audio_stream::unbind() { }
auto null_audio_stream::is_bound() const -> bool { return true; }
auto null_audio_stream::get_volume() const -> f32 { return 0; }
void null_audio_stream::set_volume(f32) { }
void null_audio_stream::put(std::span<f32 const>) { }
void null_audio_stream::flush() { }
void null_audio_stream::clear() { }
auto null_audio_stream::get() -> std::vector<f32> { return {}; }
auto null_audio_stream::available_bytes() const -> i32 { return 0; }
auto null_audio_stream::queued_bytes() const -> i32 { return 0; }

auto null_audio_system::create_output(specification const&) const -> std::unique_ptr<audio_stream>
{
    return std::make_unique<null_audio_stream>();
}

auto null_audio_system::create_input() const -> std::unique_ptr<audio_stream>
{
    return std::make_unique<null_audio_stream>();
}

}
