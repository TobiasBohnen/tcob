// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioCodec_mid.hpp"

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)

    #include <TinySoundFont/tml.h>
    #include <TinySoundFont/tsf.h>

    #include <algorithm>

    #include "tcob/core/io/Stream.hpp"

namespace tcob::audio::detail {

    #define RENDER_EFFECTSAMPLEBLOCK 64

void static handle_message(tsf* font, tml_message* midiMessage)
{
    switch (midiMessage->type) {
    case TML_PROGRAM_CHANGE: // channel program (preset) change (special handling for 10th MIDI channel with drums)
        tsf_channel_set_presetnumber(font, midiMessage->channel, midiMessage->program, (midiMessage->channel == 9));
        break;
    case TML_NOTE_ON:        // play a note
        tsf_channel_note_on(font, midiMessage->channel, midiMessage->key, static_cast<f32>(midiMessage->velocity) / 127.0f);
        break;
    case TML_NOTE_OFF:       // stop a note
        tsf_channel_note_off(font, midiMessage->channel, midiMessage->key);
        break;
    case TML_PITCH_BEND:     // pitch wheel modification
        tsf_channel_set_pitchwheel(font, midiMessage->channel, midiMessage->pitch_bend);
        break;
    case TML_CONTROL_CHANGE: // MIDI controller messages
        tsf_channel_midi_control(font, midiMessage->channel, midiMessage->control, midiMessage->control_value);
        break;
    }
}

midi_decoder::midi_decoder() = default;

midi_decoder::~midi_decoder()
{
    tml_free(_firstMessage);
}

void midi_decoder::seek_from_start(milliseconds pos)
{
    tsf_reset(_font->get_impl());
    _currentMessage = _firstMessage;
    _currentTime    = pos.count();
    while (_currentTime > _currentMessage->time) {
        handle_message(_font->get_impl(), _currentMessage);
        _currentMessage = _currentMessage->next;
    }
}

auto midi_decoder::open() -> std::optional<buffer::information>
{
    _font            = std::any_cast<assets::asset_ptr<sound_font>>(context());
    _info.SampleRate = _font->info().SampleRate;
    _info.Channels   = _font->info().Channels;

    auto const buffer {stream().read_all<byte>()};
    _firstMessage   = {tml_load_memory(buffer.data(), static_cast<i32>(buffer.size()))};
    _currentMessage = _firstMessage;
    u32 duration {0};
    tml_get_info(_firstMessage, nullptr, nullptr, nullptr, nullptr, &duration);
    _info.FrameCount = static_cast<i64>((static_cast<f32>(duration) / 1000.0f) * static_cast<f32>(_info.SampleRate));

    tsf_reset(_font->get_impl());
    return _info;
}

auto midi_decoder::decode(std::span<f32> outputSamples) -> i32
{
    i32  samplesRemaining {static_cast<i32>(outputSamples.size() / static_cast<u32>(_info.Channels))};
    i32  sampleCount {0};
    f32* dataPtr {outputSamples.data()};
    for (i32 sampleBlock {RENDER_EFFECTSAMPLEBLOCK}; samplesRemaining > 0 && _currentMessage; samplesRemaining -= sampleBlock) {
        if (samplesRemaining < 0) {
            sampleBlock += samplesRemaining;
        }
        sampleBlock = std::min(sampleBlock, samplesRemaining);

        // Loop through all MIDI messages which need to be played up until the current playback time
        for (_currentTime += sampleBlock * (1000.0 / static_cast<f64>(_info.SampleRate));
             _currentMessage && _currentTime >= _currentMessage->time;
             _currentMessage = _currentMessage->next) {
            handle_message(_font->get_impl(), _currentMessage);
        }

        // Render the block of audio samples in float format
        tsf_render_float(_font->get_impl(), dataPtr, sampleBlock, 0);
        dataPtr += sampleBlock * _info.Channels;
        sampleCount += sampleBlock;
    }

    return sampleCount;
}

}

#endif
