// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/SoundFont.hpp"

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)

    #if defined(_MSC_VER)
        #pragma warning(disable : 4201)
    #endif

    #include "tcob/core/ServiceLocator.hpp"
    #include "tcob/core/TaskManager.hpp"
    #include "tcob/core/io/FileStream.hpp"

    #include "ALObjects.hpp"

    #include <TinySoundFont/tml.h>
    #include <TinySoundFont/tsf.h>

    #include <utility>

namespace tcob::audio {

sound_font::~sound_font()
{
    if (_font) {
        reset();
        tsf_close(_font);
    }
}

auto sound_font::load(path const& file, bool stereo, i32 sampleRate) noexcept -> load_status
{
    io::ifstream fs {file};
    return load(fs, stereo, sampleRate);
}

auto sound_font::load(io::istream& stream, bool stereo, i32 sampleRate) noexcept -> load_status
{
    if (!stream) { return load_status::Error; }

    if (_font) {
        reset();
        tsf_close(_font);
    }

    auto const buffer {stream.read_all<ubyte>()};
    _font = tsf_load_memory(buffer.data(), static_cast<i32>(buffer.size()));

    // Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
    tsf_channel_set_bank_preset(_font, 9, 128, 0);

    _channels   = stereo ? 2 : 1;
    _sampleRate = sampleRate;

    // Set the SoundFont rendering output mode
    tsf_set_output(_font, stereo ? TSF_STEREO_INTERLEAVED : TSF_MONO, sampleRate, 0.0f);

    return _font != nullptr ? load_status::Ok : load_status::Error;
}

auto sound_font::load_async(path const& file, bool stereo, i32 sampleRate) noexcept -> std::future<load_status>
{
    return locate_service<task_manager>().run_async<load_status>([&, file, stereo, sampleRate]() { return load(file, stereo, sampleRate); });
}

auto sound_font::create_buffer(sound_font_commands const& commands) const -> buffer
{
    reset();

    // calculate duration
    f64 const duration {commands.get_total_duration().count() / 1000};

    std::vector<f32> samples(static_cast<usize>(duration * _sampleRate * _channels));
    f32*             ptr {samples.data()};

    // call 'apply' on all commands; then render them
    commands.render(_font, ptr, static_cast<u8>(_channels), _sampleRate);

    return buffer::Create({.Channels = _channels, .SampleRate = _sampleRate, .FrameCount = std::ssize(samples) / _channels}, samples);
}

auto sound_font::create_sound(sound_font_commands const& commands) const -> sound
{
    auto const audioData {create_buffer(commands)};
    auto const audioBuffer {std::make_shared<audio::al::al_buffer>()};
    audioBuffer->buffer_data(audioData.get_data(), _channels, _sampleRate);
    return sound {audioBuffer};
}

auto sound_font::get_preset_count() const -> i32
{
    assert(_font);
    return tsf_get_presetcount(_font);
}

auto sound_font::get_channel_count() const -> i8
{
    return _channels;
}

auto sound_font::get_sample_rate() const -> i32
{
    return _sampleRate;
}

auto sound_font::get_preset_name(i32 index) const -> string
{
    assert(_font);
    return tsf_get_presetname(_font, index);
}

auto sound_font::get_font() const -> tsf*
{
    return _font;
}

void sound_font::reset() const
{
    assert(_font);
    tsf_reset(_font);
}

////////////////////////////////////////////////////////////

note_on_command::note_on_command(i32 pi, midi_note note, f32 vel)
    : PresetIndex {pi}
    , Note {note}
    , Velocity {vel}
{
}

void note_on_command::apply(tsf* font) const
{
    tsf_note_on(font, PresetIndex, static_cast<u8>(Note), Velocity);
}

note_off_command::note_off_command(i32 pi, midi_note note)
    : PresetIndex {pi}
    , Note {note}
{
}

void note_off_command::apply(tsf* font) const
{
    tsf_note_off(font, PresetIndex, static_cast<u8>(Note));
}

void note_off_all_command::apply(tsf* font) const
{
    tsf_note_off_all(font);
}

channel_preset_index::channel_preset_index(i32 ch, i32 pi)
    : Channel {ch}
    , PresetIndex {pi}
{
}

void channel_preset_index::apply(tsf* font) const
{
    tsf_channel_set_presetindex(font, Channel, PresetIndex);
}

channel_pan::channel_pan(i32 ch, f32 pan)
    : Channel {ch}
    , Pan {pan}
{
}

void channel_pan::apply(tsf* font) const
{
    tsf_channel_set_pan(font, Channel, Pan);
}

channel_volume::channel_volume(i32 ch, f32 vol)
    : Channel {ch}
    , Volume {vol}
{
}

void channel_volume::apply(tsf* font) const
{
    tsf_channel_set_volume(font, Channel, Volume);
}

channel_pitch_wheel::channel_pitch_wheel(i32 ch, u16 pw)
    : Channel {ch}
    , PitchWheel {pw}
{
}

void channel_pitch_wheel::apply(tsf* font) const
{
    tsf_channel_set_pitchwheel(font, Channel, PitchWheel);
}

channel_pitch_range::channel_pitch_range(i32 ch, f32 pr)
    : Channel {ch}
    , PitchRange {pr}
{
}

void channel_pitch_range::apply(tsf* font) const
{
    tsf_channel_set_pitchrange(font, Channel, PitchRange);
}

channel_tunning::channel_tunning(i32 ch, f32 tunning)
    : Channel {ch}
    , Tunning {tunning}
{
}

void channel_tunning::apply(tsf* font) const
{
    tsf_channel_set_tuning(font, Channel, Tunning);
}

channel_note_on_command::channel_note_on_command(i32 ch, midi_note note, f32 vel)
    : Channel {ch}
    , Note {note}
    , Velocity {vel}
{
}

void channel_note_on_command::apply(tsf* font) const
{
    tsf_channel_note_on(font, Channel, static_cast<u8>(Note), Velocity);
}

channel_note_off_command::channel_note_off_command(i32 ch, midi_note note)
    : Channel {ch}
    , Note {note}
{
}

void channel_note_off_command::apply(tsf* font) const
{
    tsf_channel_note_off(font, Channel, static_cast<u8>(Note));
}

channel_note_off_all_command::channel_note_off_all_command(i32 ch)
    : Channel {ch}
{
}

void channel_note_off_all_command::apply(tsf* font) const
{
    tsf_channel_note_off_all(font, Channel);
}

channel_sound_off_all_command::channel_sound_off_all_command(i32 ch)
    : Channel {ch}
{
}

void channel_sound_off_all_command::apply(tsf* font) const
{
    tsf_channel_sounds_off_all(font, Channel);
}

////////////////////////////////////////////////////////////

void sound_font_commands::start_new_section(milliseconds duration)
{
    _totalDuration += duration;
    _commands.emplace_back(duration, std::vector<std::unique_ptr<sound_font_command>> {});
}

auto sound_font_commands::get_total_duration() const -> milliseconds
{
    return _totalDuration;
}

void sound_font_commands::render(tsf* font, f32* buffer, u8 channels, i32 sampleRate) const
{
    for (auto const& command : _commands) {
        for (auto const& subcommand : command.second) {
            subcommand->apply(font);
        }

        i32 const frameCount {static_cast<i32>(command.first.count() / 1000 * sampleRate)};
        if (frameCount == 0) {
            continue;
        }

        tsf_render_float(font, buffer, frameCount);
        buffer += frameCount * channels;
    }
}

////////////////////////////////////////////////////////////

} // namespace audio

#endif
