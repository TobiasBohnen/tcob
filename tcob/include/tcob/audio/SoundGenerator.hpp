// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Sound.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class sound_wave {
public:
    enum class type : u8 {
        Square   = 0,
        Sawtooth = 1,
        Sine     = 2,
        Noise    = 3,
        Triangle = 4
    };

    u64 RandomSeed {};

    i32 SampleRate {44100};

    // Wave type (square, sawtooth, sine, noise)
    type WaveType {type::Square};

    // Wave envelope parameters
    f32 AttackTime {0.0f};   // 0 to 1
    f32 SustainTime {0.3f};  // 0 to 1
    f32 SustainPunch {0.0f}; // 0 to 1
    f32 DecayTime {0.4f};    // 0 to 1

    // Frequency parameters
    f32 StartFrequency {0.3f}; // 0 to 1
    f32 MinFrequency {0.0f};   // 0 to 1
    f32 Slide {0.0f};          // -1 to 1
    f32 DeltaSlide {0.0f};     // -1 to 1
    f32 VibratoDepth {0.0f};   // 0 to 1
    f32 VibratoSpeed {0.0f};   // 0 to 1

    // Tone change parameters
    f32 ChangeAmount {0.0f}; // -1 to 1
    f32 ChangeSpeed {0.0f};  // 0 to 1

    // Square wave parameters
    f32 SquareDuty {0.0f}; // 0 to 1
    f32 DutySweep {0.0f};  // -1 to 1

    // Repeat parameters
    f32 RepeatSpeed {0.0f}; // 0 to 1

    // Phaser parameters
    f32 PhaserOffset {0.0f}; // -1 to 1
    f32 PhaserSweep {0.0f};  // -1 to 1

    // Filter parameters
    f32 LowPassFilterCutoff {1.0f};       // 0 to 1
    f32 LowPassFilterCutoffSweep {0.0f};  // -1 to 1
    f32 LowPassFilterResonance {0.0f};    // 0 to 1
    f32 HighPassFilterCutoff {0.0f};      // 0 to 1
    f32 HighPassFilterCutoffSweep {0.0f}; // -1 to 1

    void static Serialize(sound_wave const& v, auto&& s);
    auto static Deserialize(sound_wave& v, auto&& s) -> bool;
};

inline void sound_wave::Serialize(sound_wave const& v, auto&& s)
{
    s["random_seed"]      = v.RandomSeed;
    s["sample_rate"]      = v.SampleRate;
    s["wave_type"]        = v.WaveType;
    s["attack_time"]      = v.AttackTime;
    s["sustain_time"]     = v.SustainTime;
    s["sustain_punch"]    = v.SustainPunch;
    s["decay_time"]       = v.DecayTime;
    s["start_frequency"]  = v.StartFrequency;
    s["min_frequency"]    = v.MinFrequency;
    s["slide"]            = v.Slide;
    s["delta_slide"]      = v.DeltaSlide;
    s["vibrato_depth"]    = v.VibratoDepth;
    s["vibrato_speed"]    = v.VibratoSpeed;
    s["change_amount"]    = v.ChangeAmount;
    s["change_speed"]     = v.ChangeSpeed;
    s["square_duty"]      = v.SquareDuty;
    s["duty_sweep"]       = v.DutySweep;
    s["repeat_speed"]     = v.RepeatSpeed;
    s["phaser_offset"]    = v.PhaserOffset;
    s["phaser_sweep"]     = v.PhaserSweep;
    s["lpf_cutoff"]       = v.LowPassFilterCutoff;
    s["lpf_cutoff_sweep"] = v.LowPassFilterCutoffSweep;
    s["lpf_resonance"]    = v.LowPassFilterResonance;
    s["hpf_cutoff"]       = v.HighPassFilterCutoff;
    s["hpf_cutoff_sweep"] = v.HighPassFilterCutoffSweep;
}

inline auto sound_wave::Deserialize(sound_wave& v, auto&& s) -> bool
{
    return s.try_get(v.RandomSeed, "random_seed")
        && s.try_get(v.SampleRate, "sample_rate")
        && s.try_get(v.WaveType, "wave_type")
        && s.try_get(v.AttackTime, "attack_time")
        && s.try_get(v.SustainTime, "sustain_time")
        && s.try_get(v.SustainPunch, "sustain_punch")
        && s.try_get(v.DecayTime, "decay_time")
        && s.try_get(v.StartFrequency, "start_frequency")
        && s.try_get(v.MinFrequency, "min_frequency")
        && s.try_get(v.Slide, "slide")
        && s.try_get(v.DeltaSlide, "delta_slide")
        && s.try_get(v.VibratoDepth, "vibrato_depth")
        && s.try_get(v.VibratoSpeed, "vibrato_speed")
        && s.try_get(v.ChangeAmount, "change_amount")
        && s.try_get(v.ChangeSpeed, "change_speed")
        && s.try_get(v.SquareDuty, "square_duty")
        && s.try_get(v.DutySweep, "duty_sweep")
        && s.try_get(v.RepeatSpeed, "repeat_speed")
        && s.try_get(v.PhaserOffset, "phaser_offset")
        && s.try_get(v.PhaserSweep, "phaser_sweep")
        && s.try_get(v.LowPassFilterCutoff, "lpf_cutoff")
        && s.try_get(v.LowPassFilterCutoffSweep, "lpf_cutoff_sweep")
        && s.try_get(v.LowPassFilterResonance, "lpf_resonance")
        && s.try_get(v.HighPassFilterCutoff, "hpf_cutoff")
        && s.try_get(v.HighPassFilterCutoffSweep, "hpf_cutoff_sweep");
}

////////////////////////////////////////////////////////////

class TCOB_API sound_generator final {
public:
    sound_generator() = default;
    explicit sound_generator(random::rng_split_mix_64 random);

    auto generate_pickup_coin() -> sound_wave;
    auto generate_laser_shoot() -> sound_wave;
    auto generate_explosion() -> sound_wave;
    auto generate_powerup() -> sound_wave;
    auto generate_hit_hurt() -> sound_wave;
    auto generate_jump() -> sound_wave;
    auto generate_blip_select() -> sound_wave;
    auto generate_random() -> sound_wave;

    auto mutate_wave(sound_wave const& wave) -> sound_wave;
    auto sanitize_wave(sound_wave const& wave) -> sound_wave;

    auto create_buffer [[nodiscard]] (sound_wave const& wave) -> buffer;
    auto create_sound [[nodiscard]] (sound_wave const& wave) -> sound;

private:
    random::rng_split_mix_64 _random;
};
}
