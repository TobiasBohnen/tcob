// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <tuple>

#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Serialization.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API sound_wave {
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

    void sanitize();

    auto operator==(sound_wave const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&sound_wave::RandomSeed> {"random_seed"},
            member<&sound_wave::SampleRate, 44100> {"sample_rate"},
            member<&sound_wave::WaveType, type::Square> {"wave_type"},
            member<&sound_wave::AttackTime, 0.0f> {"attack_time"},
            member<&sound_wave::SustainTime, 0.3f> {"sustain_time"},
            member<&sound_wave::SustainPunch, 0.0f> {"sustain_punch"},
            member<&sound_wave::DecayTime, 0.4f> {"decay_time"},
            member<&sound_wave::StartFrequency, 0.3f> {"start_frequency"},
            member<&sound_wave::MinFrequency, 0.0f> {"min_frequency"},
            member<&sound_wave::Slide, 0.0f> {"slide"},
            member<&sound_wave::DeltaSlide, 0.0f> {"delta_slide"},
            member<&sound_wave::VibratoDepth, 0.0f> {"vibrato_depth"},
            member<&sound_wave::VibratoSpeed, 0.0f> {"vibrato_speed"},
            member<&sound_wave::ChangeAmount, 0.0f> {"change_amount"},
            member<&sound_wave::ChangeSpeed, 0.0f> {"change_speed"},
            member<&sound_wave::SquareDuty, 0.0f> {"square_duty"},
            member<&sound_wave::DutySweep, 0.0f> {"duty_sweep"},
            member<&sound_wave::RepeatSpeed, 0.0f> {"repeat_speed"},
            member<&sound_wave::PhaserOffset, 0.0f> {"phaser_offset"},
            member<&sound_wave::PhaserSweep, 0.0f> {"phaser_sweep"},
            member<&sound_wave::LowPassFilterCutoff, 1.0f> {"lpf_cutoff"},
            member<&sound_wave::LowPassFilterCutoffSweep, 0.0f> {"lpf_cutoff_sweep"},
            member<&sound_wave::LowPassFilterResonance, 0.0f> {"lpf_resonance"},
            member<&sound_wave::HighPassFilterCutoff, 0.0f> {"hpf_cutoff"},
            member<&sound_wave::HighPassFilterCutoffSweep, 0.0f> {"hpf_cutoff_sweep"}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API sound_generator final {
public:
    sound_generator() = default;

    auto generate_pickup_coin [[nodiscard]] (u64 seed) -> sound_wave;
    auto generate_laser_shoot [[nodiscard]] (u64 seed) -> sound_wave;
    auto generate_explosion [[nodiscard]] (u64 seed) -> sound_wave;
    auto generate_powerup [[nodiscard]] (u64 seed) -> sound_wave;
    auto generate_hit_hurt [[nodiscard]] (u64 seed) -> sound_wave;
    auto generate_jump [[nodiscard]] (u64 seed) -> sound_wave;
    auto generate_blip_select [[nodiscard]] (u64 seed) -> sound_wave;
    auto generate_random [[nodiscard]] (u64 seed) -> sound_wave;

    auto mutate_wave [[nodiscard]] (u64 seed, sound_wave const& wave) -> sound_wave;

    auto create_buffer [[nodiscard]] (sound_wave const& wave) -> buffer;
};
}
