// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <tuple>
#include <utility>

#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Sound.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API [[nodiscard]] sound_wave {
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

    void sanitize();

    auto operator==(sound_wave const& other) const -> bool = default;

    auto constexpr static Members()
    {
        return std::tuple {
            std::pair {"random_seed", &sound_wave::RandomSeed},
            std::pair {"sample_rate", &sound_wave::SampleRate},
            std::pair {"wave_type", &sound_wave::WaveType},
            std::pair {"attack_time", &sound_wave::AttackTime},
            std::pair {"sustain_time", &sound_wave::SustainTime},
            std::pair {"sustain_punch", &sound_wave::SustainPunch},
            std::pair {"decay_time", &sound_wave::DecayTime},
            std::pair {"start_frequency", &sound_wave::StartFrequency},
            std::pair {"min_frequency", &sound_wave::MinFrequency},
            std::pair {"slide", &sound_wave::Slide},
            std::pair {"delta_slide", &sound_wave::DeltaSlide},
            std::pair {"vibrato_depth", &sound_wave::VibratoDepth},
            std::pair {"vibrato_speed", &sound_wave::VibratoSpeed},
            std::pair {"change_amount", &sound_wave::ChangeAmount},
            std::pair {"change_speed", &sound_wave::ChangeSpeed},
            std::pair {"square_duty", &sound_wave::SquareDuty},
            std::pair {"duty_sweep", &sound_wave::DutySweep},
            std::pair {"repeat_speed", &sound_wave::RepeatSpeed},
            std::pair {"phaser_offset", &sound_wave::PhaserOffset},
            std::pair {"phaser_sweep", &sound_wave::PhaserSweep},
            std::pair {"lpf_cutoff", &sound_wave::LowPassFilterCutoff},
            std::pair {"lpf_cutoff_sweep", &sound_wave::LowPassFilterCutoffSweep},
            std::pair {"lpf_resonance", &sound_wave::LowPassFilterResonance},
            std::pair {"hpf_cutoff", &sound_wave::HighPassFilterCutoff},
            std::pair {"hpf_cutoff_sweep", &sound_wave::HighPassFilterCutoffSweep}};
    }
};

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

    auto create_buffer [[nodiscard]] (sound_wave const& wave) -> buffer;
    auto create_sound [[nodiscard]] (sound_wave const& wave) -> std::shared_ptr<sound>;

private:
    random::rng_split_mix_64 _random;
};
}
