// Copyright (c) 2024 Tobias Bohnen
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
    auto create_sound [[nodiscard]] (sound_wave const& wave) -> sound;

private:
    random::rng_split_mix_64 _random;
};
}
