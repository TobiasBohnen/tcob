// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)

    #include <vector>

    #include "tcob/audio/Sound.hpp"
    #include "tcob/core/Interfaces.hpp"

struct tsf;
struct tml_message;

namespace tcob::audio {
////////////////////////////////////////////////////////////

class sound_font;
class sound_font_command;
class sound_font_commands;

////////////////////////////////////////////////////////////

class TCOB_API sound_font final : public non_copyable {
public:
    sound_font() = default;
    ~sound_font();

    auto get_preset_count() const -> i32; // TODO: get_
    auto get_channel_count() const -> i8; // TODO: get_
    auto get_sample_rate() const -> i32;  // TODO: get_

    auto load [[nodiscard]] (path const& file, bool stereo = true, i32 sampleRate = 44100) noexcept -> load_status;
    auto load [[nodiscard]] (io::istream& stream, bool stereo = true, i32 sampleRate = 44100) noexcept -> load_status;
    auto load_async [[nodiscard]] (path const& file, bool stereo = true, i32 sampleRate = 44100) noexcept -> std::future<load_status>;

    auto create_buffer [[nodiscard]] (sound_font_commands const& commands) const -> buffer;
    auto create_sound [[nodiscard]] (sound_font_commands const& commands) const -> sound;

    auto get_preset_name(i32 index) const -> string;

    auto get_font() const -> tsf*;

    static inline char const* asset_name {"sound_font"};

private:
    void reset() const;
    void render(milliseconds duration, f32*& ptr) const;

    tsf* _font {nullptr};
    i8   _channels {0};
    i32  _sampleRate {0};
};

////////////////////////////////////////////////////////////

class TCOB_API sound_font_commands : public non_copyable {
    friend class sound_font;

public:
    auto get_total_duration() const -> milliseconds;

    void start_new_section(milliseconds duration);

    template <std::derived_from<sound_font_command> T>
    void add(auto&&... args);

private:
    void render(tsf* font, f32* buffer, u8 channels, i32 sampleRate) const;

    milliseconds _totalDuration {0};

    std::vector<std::pair<milliseconds, std::vector<std::unique_ptr<sound_font_command>>>> _commands;
};

////////////////////////////////////////////////////////////

enum class midi_note : u8 {
    CNeg1 = 0,
    C0    = CNeg1 + 12,
    C1    = C0 + 12,
    C2    = C1 + 12,
    C3    = C2 + 12,
    C4    = C3 + 12,
    C5    = C4 + 12,
    C6    = C5 + 12,
    C7    = C6 + 12,
    C8    = C7 + 12,
    C9    = C8 + 12,

    CSharpNeg1 = 1,
    CSharp0    = CSharpNeg1 + 12,
    CSharp1    = CSharp0 + 12,
    CSharp2    = CSharp1 + 12,
    CSharp3    = CSharp2 + 12,
    CSharp4    = CSharp3 + 12,
    CSharp5    = CSharp4 + 12,
    CSharp6    = CSharp5 + 12,
    CSharp7    = CSharp6 + 12,
    CSharp8    = CSharp7 + 12,
    CSharp9    = CSharp8 + 12,

    DNeg1 = 2,
    D0    = DNeg1 + 12,
    D1    = D0 + 12,
    D2    = D1 + 12,
    D3    = D2 + 12,
    D4    = D3 + 12,
    D5    = D4 + 12,
    D6    = D5 + 12,
    D7    = D6 + 12,
    D8    = D7 + 12,
    D9    = D8 + 12,

    DSharpNeg1 = 3,
    DSharp0    = DSharpNeg1 + 12,
    DSharp1    = DSharp0 + 12,
    DSharp2    = DSharp1 + 12,
    DSharp3    = DSharp2 + 12,
    DSharp4    = DSharp3 + 12,
    DSharp5    = DSharp4 + 12,
    DSharp6    = DSharp5 + 12,
    DSharp7    = DSharp6 + 12,
    DSharp8    = DSharp7 + 12,
    DSharp9    = DSharp8 + 12,

    ENeg1 = 4,
    E0    = ENeg1 + 12,
    E1    = E0 + 12,
    E2    = E1 + 12,
    E3    = E2 + 12,
    E4    = E3 + 12,
    E5    = E4 + 12,
    E6    = E5 + 12,
    E7    = E6 + 12,
    E8    = E7 + 12,
    E9    = E8 + 12,

    FNeg1 = 5,
    F0    = FNeg1 + 12,
    F1    = F0 + 12,
    F2    = F1 + 12,
    F3    = F2 + 12,
    F4    = F3 + 12,
    F5    = F4 + 12,
    F6    = F5 + 12,
    F7    = F6 + 12,
    F8    = F7 + 12,
    F9    = F8 + 12,

    FSharpNeg1 = 6,
    FSharp0    = FSharpNeg1 + 12,
    FSharp1    = FSharp0 + 12,
    FSharp2    = FSharp1 + 12,
    FSharp3    = FSharp2 + 12,
    FSharp4    = FSharp3 + 12,
    FSharp5    = FSharp4 + 12,
    FSharp6    = FSharp5 + 12,
    FSharp7    = FSharp6 + 12,
    FSharp8    = FSharp7 + 12,
    FSharp9    = FSharp8 + 12,

    GNeg1 = 7,
    G0    = GNeg1 + 12,
    G1    = G0 + 12,
    G2    = G1 + 12,
    G3    = G2 + 12,
    G4    = G3 + 12,
    G5    = G4 + 12,
    G6    = G5 + 12,
    G7    = G6 + 12,
    G8    = G7 + 12,
    G9    = G8 + 12,

    GSharpNeg1 = 8,
    GSharp0    = GSharpNeg1 + 12,
    GSharp1    = GSharp0 + 12,
    GSharp2    = GSharp1 + 12,
    GSharp3    = GSharp2 + 12,
    GSharp4    = GSharp3 + 12,
    GSharp5    = GSharp4 + 12,
    GSharp6    = GSharp5 + 12,
    GSharp7    = GSharp6 + 12,
    GSharp8    = GSharp7 + 12,

    ANeg1 = 9,
    A0    = ANeg1 + 12,
    A1    = A0 + 12,
    A2    = A1 + 12,
    A3    = A2 + 12,
    A4    = A3 + 12,
    A5    = A4 + 12,
    A6    = A5 + 12,
    A7    = A6 + 12,
    A8    = A7 + 12,

    ASharpNeg1 = 10,
    ASharp0    = ASharpNeg1 + 12,
    ASharp1    = ASharp0 + 12,
    ASharp2    = ASharp1 + 12,
    ASharp3    = ASharp2 + 12,
    ASharp4    = ASharp3 + 12,
    ASharp5    = ASharp4 + 12,
    ASharp6    = ASharp5 + 12,
    ASharp7    = ASharp6 + 12,
    ASharp8    = ASharp7 + 12,

    BNeg1 = 11,
    B0    = BNeg1 + 12,
    B1    = B0 + 12,
    B2    = B1 + 12,
    B3    = B2 + 12,
    B4    = B3 + 12,
    B5    = B4 + 12,
    B6    = B5 + 12,
    B7    = B6 + 12,
    B8    = B7 + 12
};

class TCOB_API sound_font_command {
public:
    sound_font_command()                                                            = default;
    sound_font_command(sound_font_command const& other) noexcept                    = delete;
    auto operator=(sound_font_command const& other) noexcept -> sound_font_command& = delete;
    sound_font_command(sound_font_command&& other) noexcept                         = delete;
    auto operator=(sound_font_command&& other) noexcept -> sound_font_command&      = delete;
    virtual ~sound_font_command()                                                   = default;

    void virtual apply(tsf* font) const = 0;
};

class TCOB_API note_on_command : public sound_font_command {
public:
    note_on_command(i32 pi, midi_note note, f32 vel);

    i32       PresetIndex {0};
    midi_note Note {0};
    f32       Velocity {0};

    void apply(tsf* font) const override;
};

class TCOB_API note_off_command : public sound_font_command {
public:
    note_off_command(i32 pi, midi_note note);

    i32       PresetIndex {0};
    midi_note Note {0};

    void apply(tsf* font) const override;
};

class TCOB_API note_off_all_command : public sound_font_command {
public:
    void apply(tsf* font) const override;
};

class TCOB_API channel_preset_index : public sound_font_command {
public:
    channel_preset_index(i32 ch, i32 pi);

    i32 Channel {0};
    i32 PresetIndex {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_pan : public sound_font_command {
public:
    channel_pan(i32 ch, f32 pan);

    i32 Channel {0};
    f32 Pan {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_volume : public sound_font_command {
public:
    channel_volume(i32 ch, f32 vol);

    i32 Channel {0};
    f32 Volume {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_pitch_wheel : public sound_font_command {
public:
    channel_pitch_wheel(i32 ch, u16 pw);

    i32 Channel {0};
    u16 PitchWheel {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_pitch_range : public sound_font_command {
public:
    channel_pitch_range(i32 ch, f32 pr);

    i32 Channel {0};
    f32 PitchRange {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_tunning : public sound_font_command {
public:
    channel_tunning(i32 ch, f32 tunning);

    i32 Channel {0};
    f32 Tunning {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_note_on_command : public sound_font_command {
public:
    channel_note_on_command(i32 ch, midi_note note, f32 vel);

    i32       Channel {0};
    midi_note Note {0};
    f32       Velocity {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_note_off_command : public sound_font_command {
public:
    channel_note_off_command(i32 ch, midi_note note);

    i32       Channel {0};
    midi_note Note {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_note_off_all_command : public sound_font_command {
public:
    channel_note_off_all_command(i32 ch);

    i32 Channel {0};

    void apply(tsf* font) const override;
};

class TCOB_API channel_sound_off_all_command : public sound_font_command {
public:
    channel_sound_off_all_command(i32 ch);

    i32 Channel {0};

    void apply(tsf* font) const override;
};

}

    #include "SoundFont.inl"

#endif
