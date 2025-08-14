// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <tuple>

#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Sound.hpp"
#include "tcob/core/Serialization.hpp"
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

    auto static constexpr Members()
    {
        return std::tuple {
            member<&sound_wave::RandomSeed> {"random_seed"},
            member<&sound_wave::SampleRate> {"sample_rate"},
            member<&sound_wave::WaveType> {"wave_type"},
            member<&sound_wave::AttackTime> {"attack_time"},
            member<&sound_wave::SustainTime> {"sustain_time"},
            member<&sound_wave::SustainPunch> {"sustain_punch"},
            member<&sound_wave::DecayTime> {"decay_time"},
            member<&sound_wave::StartFrequency> {"start_frequency"},
            member<&sound_wave::MinFrequency> {"min_frequency"},
            member<&sound_wave::Slide> {"slide"},
            member<&sound_wave::DeltaSlide> {"delta_slide"},
            member<&sound_wave::VibratoDepth> {"vibrato_depth"},
            member<&sound_wave::VibratoSpeed> {"vibrato_speed"},
            member<&sound_wave::ChangeAmount> {"change_amount"},
            member<&sound_wave::ChangeSpeed> {"change_speed"},
            member<&sound_wave::SquareDuty> {"square_duty"},
            member<&sound_wave::DutySweep> {"duty_sweep"},
            member<&sound_wave::RepeatSpeed> {"repeat_speed"},
            member<&sound_wave::PhaserOffset> {"phaser_offset"},
            member<&sound_wave::PhaserSweep> {"phaser_sweep"},
            member<&sound_wave::LowPassFilterCutoff> {"lpf_cutoff"},
            member<&sound_wave::LowPassFilterCutoffSweep> {"lpf_cutoff_sweep"},
            member<&sound_wave::LowPassFilterResonance> {"lpf_resonance"},
            member<&sound_wave::HighPassFilterCutoff> {"hpf_cutoff"},
            member<&sound_wave::HighPassFilterCutoffSweep> {"hpf_cutoff_sweep"}};
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
