// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on: sfxr

#include "tcob/audio/synth/SoundGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "SoundGenerator_private.hpp"

#include "tcob/audio/Buffer.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::audio {

////////////////////////////////////////////////////////////

// Generate sound: Pickup/Coin
auto sound_generator::generate_pickup_coin(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    retValue.StartFrequency = random(0.4f, 0.9f);
    retValue.AttackTime     = 0.0f;
    retValue.SustainTime    = random(0.0f, 0.1f);
    retValue.DecayTime      = random(0.1f, 0.5f);
    retValue.SustainPunch   = random(0.3f, 0.6f);

    if (random(0, 1) == 1) {
        retValue.ChangeSpeed  = random(0.5f, 0.7f);
        retValue.ChangeAmount = random(0.2f, 0.6f);
    }

    return retValue;
}

// Generate sound: Laser shoot
auto sound_generator::generate_laser_shoot(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    retValue.WaveType = static_cast<sound_wave::type>(random(0, 2));

    if ((retValue.WaveType == sound_wave::type::Sine) && random(0, 1) == 1) {
        retValue.WaveType = static_cast<sound_wave::type>(random(0, 1));
    }

    retValue.StartFrequency = random(0.5f, 1.0f);
    retValue.MinFrequency   = std::max(retValue.StartFrequency - random(0.2f, 0.8f), 0.2f);

    retValue.Slide = -random(-0.35f, -0.15f);

    if (random(0, 2) == 0) {
        retValue.StartFrequency = random(0.3f, 0.9f);
        retValue.MinFrequency   = random(0.0f, 0.1f);
        retValue.Slide          = random(-0.35f, -0.05f);
    }

    if (random(0, 1) == 1) {
        retValue.SquareDuty = random(0.0f, 0.5f);
        retValue.DutySweep  = random(0.0f, 0.2f);
    } else {
        retValue.SquareDuty = random(0.4f, 0.9f);
        retValue.DutySweep  = -random(0.0f, 0.7f);
    }

    retValue.AttackTime  = 0.0f;
    retValue.SustainTime = random(0.1f, 0.3f);
    retValue.DecayTime   = random(0.0f, 0.4f);

    if (random(0, 1) == 1) {
        retValue.SustainPunch = random(0.0f, 0.3f);
    }

    if (random(0, 2) == 0) {
        retValue.PhaserOffset = random(0.0f, 0.2f);
        retValue.PhaserSweep  = -random(0.0f, 0.2f);
    }

    if (random(0, 1) == 1) {
        retValue.HighPassFilterCutoff = random(0.0f, 0.3f);
    }

    return retValue;
}

// Generate sound: Explosion
auto sound_generator::generate_explosion(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    retValue.WaveType = sound_wave::type::Noise;

    if (random(0, 1) == 1) {
        retValue.StartFrequency = random(0.1f, 0.5f);
        retValue.Slide          = random(-0.1f, 0.3f);
    } else {
        retValue.StartFrequency = random(0.2f, 0.7f);
        retValue.Slide          = random(-0.4f, -0.2f);
    }

    retValue.StartFrequency *= retValue.StartFrequency;

    if (random(0, 4) == 0) {
        retValue.Slide = 0.0f;
    }
    if (random(0, 2) == 0) {
        retValue.RepeatSpeed = random(0.3f, 0.8f);
    }

    retValue.AttackTime  = 0.0f;
    retValue.SustainTime = random(0.1f, 0.4f);
    retValue.DecayTime   = random(0.0f, 0.5f);

    if (random(0, 1) == 1) {
        retValue.PhaserOffset = random(-0.3f, 0.6f);
        retValue.PhaserSweep  = -random(0.0f, 0.3f);
    }

    retValue.SustainPunch = random(0.2f, 0.8f);

    if (random(0, 1) == 1) {
        retValue.VibratoDepth = random(0.0f, 0.7f);
        retValue.VibratoSpeed = random(0.0f, 0.6f);
    }

    if (random(0, 2) == 0) {
        retValue.ChangeSpeed  = random(0.6f, 0.9f);
        retValue.ChangeAmount = random(-0.8f, 0.8f);
    }

    return retValue;
}

// Generate sound: Powerup
auto sound_generator::generate_powerup(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    if (random(0, 1) == 1) {
        retValue.WaveType = sound_wave::type::Sawtooth;
    } else {
        retValue.SquareDuty = random(0.0f, 0.6f);
    }

    if (random(0, 1) == 1) {
        retValue.StartFrequency = random(0.2f, 0.5f);
        retValue.Slide          = random(0.1f, 0.5f);
        retValue.RepeatSpeed    = random(0.4f, 0.8f);
    } else {
        retValue.StartFrequency = random(0.5f, 0.5f);
        retValue.Slide          = random(0.05f, 0.205f);

        if (random(0, 1) == 1) {
            retValue.VibratoDepth = random(0.0f, 0.7f);
            retValue.VibratoSpeed = random(0.0f, 0.6f);
        }
    }

    retValue.AttackTime  = 0.0f;
    retValue.SustainTime = random(0.0f, 0.4f);
    retValue.DecayTime   = random(0.1f, 0.5f);

    return retValue;
}

// Generate sound: Hit/Hurt
auto sound_generator::generate_hit_hurt(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    retValue.WaveType = static_cast<sound_wave::type>(random(0, 2));
    if (retValue.WaveType == sound_wave::type::Sine) {
        retValue.WaveType = sound_wave::type::Noise;
    }
    if (retValue.WaveType == sound_wave::type::Square) {
        retValue.SquareDuty = random(0.0f, 0.6f);
    }

    retValue.StartFrequency = random(0.2f, 0.8f);
    retValue.Slide          = random(-0.7f, -0.3f);
    retValue.AttackTime     = 0.0f;
    retValue.SustainTime    = random(0.0f, 0.1f);
    retValue.DecayTime      = random(0.1f, 0.3f);

    if (random(0, 1) == 1) {
        retValue.HighPassFilterCutoff = random(0.0f, 0.3f);
    }

    return retValue;
}

// Generate sound: Jump
auto sound_generator::generate_jump(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    retValue.WaveType       = sound_wave::type::Square;
    retValue.SquareDuty     = random(0.0f, 0.6f);
    retValue.StartFrequency = random(0.3f, 0.6f);
    retValue.Slide          = random(0.1f, 0.3f);
    retValue.AttackTime     = 0.0f;
    retValue.SustainTime    = random(0.1f, 0.4f);
    retValue.DecayTime      = random(0.1f, 0.3f);

    if (random(0, 1) == 1) {
        retValue.HighPassFilterCutoff = random(0.0f, 0.3f);
    }
    if (random(0, 1) == 1) {
        retValue.LowPassFilterCutoff = random(0.4f, 1.0f);
    }

    return retValue;
}

// Generate sound: Blip/Select
auto sound_generator::generate_blip_select(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    retValue.WaveType = static_cast<sound_wave::type>(random(0, 1));
    if (retValue.WaveType == sound_wave::type::Square) {
        retValue.SquareDuty = random(0.0f, 0.6f);
    }
    retValue.StartFrequency       = random(0.2f, 0.6f);
    retValue.AttackTime           = 0.0f;
    retValue.SustainTime          = random(0.1f, 0.2f);
    retValue.DecayTime            = random(0.0f, 0.2f);
    retValue.HighPassFilterCutoff = 0.1f;

    return retValue;
}

// Generate random sound
auto sound_generator::generate_random(u64 seed) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {};
    retValue.RandomSeed = random.state()[0];

    retValue.StartFrequency = std::pow(random(-1.0f, 1.0f), 2.0f);

    if (random(0, 1) == 1) {
        retValue.StartFrequency = std::pow(random(-1.0f, 1.0f), 3.0f) + 0.5f;
    }

    retValue.MinFrequency = 0.0f;
    retValue.Slide        = std::pow(random(-1.0f, 1.0f), 5.0f);

    if ((retValue.StartFrequency > 0.7f) && (retValue.Slide > 0.2f)) {
        retValue.Slide = -retValue.Slide;
    }
    if ((retValue.StartFrequency < 0.2f) && (retValue.Slide < -0.05f)) {
        retValue.Slide = -retValue.Slide;
    }

    retValue.DeltaSlide   = std::pow(random(-1.0f, 1.0f), 3.0f);
    retValue.SquareDuty   = random(-1.0f, 1.0f);
    retValue.DutySweep    = std::pow(random(-1.0f, 1.0f), 3.0f);
    retValue.VibratoDepth = std::pow(random(-1.0f, 1.0f), 3.0f);
    retValue.VibratoSpeed = random(-1.0f, 1.0f);
    retValue.AttackTime   = std::pow(random(-1.0f, 1.0f), 3.0f);
    retValue.SustainTime  = std::pow(random(-1.0f, 1.0f), 2.0f);
    retValue.DecayTime    = random(-1.0f, 1.0f);
    retValue.SustainPunch = std::pow(random(0.0f, 0.8f), 2.0f);

    if (retValue.AttackTime + retValue.SustainTime + retValue.DecayTime < 0.2f) {
        retValue.SustainTime += 0.2f + random(0.0f, 0.3f);
        retValue.DecayTime += 0.2f + random(0.0f, 0.3f);
    }

    retValue.LowPassFilterResonance   = random(-1.0f, 1.0f);
    retValue.LowPassFilterCutoff      = 1.0f - std::pow(random(0.0f, 1.0f), 3.0f);
    retValue.LowPassFilterCutoffSweep = std::pow(random(-1.0f, 1.0f), 3.0f);

    if (retValue.LowPassFilterCutoff < 0.1f && retValue.LowPassFilterCutoffSweep < -0.05f) {
        retValue.LowPassFilterCutoffSweep = -retValue.LowPassFilterCutoffSweep;
    }

    retValue.HighPassFilterCutoff      = std::pow(random(0.0f, 1.0f), 5.0f);
    retValue.HighPassFilterCutoffSweep = std::pow(random(-1.0f, 1.0f), 5.0f);
    retValue.PhaserOffset              = std::pow(random(-1.0f, 1.0f), 3.0f);
    retValue.PhaserSweep               = std::pow(random(-1.0f, 1.0f), 3.0f);
    retValue.RepeatSpeed               = random(-1.0f, 1.0f);
    retValue.ChangeSpeed               = random(-1.0f, 1.0f);
    retValue.ChangeAmount              = random(-1.0f, 1.0f);

    retValue.sanitize();
    return retValue;
}

// Mutate current sound
auto sound_generator::mutate_wave(u64 seed, sound_wave const& wave) -> sound_wave
{
    random::prng_split_mix_64 random {seed};

    sound_wave retValue {wave};

    if (random(0, 1) == 1) { retValue.StartFrequency += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.Slide += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.DeltaSlide += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.SquareDuty += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.DutySweep += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.VibratoDepth += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.VibratoSpeed += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.AttackTime += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.SustainTime += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.DecayTime += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.SustainPunch += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.LowPassFilterResonance += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.LowPassFilterCutoff += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.LowPassFilterCutoffSweep += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.HighPassFilterCutoff += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.HighPassFilterCutoffSweep += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.PhaserOffset += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.PhaserSweep += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.RepeatSpeed += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.ChangeSpeed += random(-0.05f, 0.05f); }
    if (random(0, 1) == 1) { retValue.ChangeAmount += random(-0.05f, 0.05f); }

    retValue.sanitize();
    return retValue;
}

// Generates new wave from wave parameters
// NOTE: By default wave is generated as 44100Hz, 32bit float, mono
auto sound_generator::create_buffer(sound_wave const& wave) -> buffer
{
    constexpr i32 MAX_WAVE_LENGTH_SECONDS {10};   // Max length for wave: 10 seconds
    constexpr i32 MAX_SUPERSAMPLING {8};
    constexpr f32 SAMPLE_SCALE_COEFICIENT {0.2f}; // NOTE: Used to scale sample value to [-1..1]

    i32       repeatTime {0};
    i32 const repeatLimit {wave.RepeatSpeed == 0.0f
                               ? 0
                               : static_cast<i32>((std::pow(1.0f - wave.RepeatSpeed, 2.0f) * 20000) + 32)};

    square_duty squareDuty;
    period      wavePeriod;
    auto const  reset {[&] {
        wavePeriod = {wave};
        squareDuty = {wave};
    }};

    reset();

    filter   filter {wave};
    vibrato  vibrato {wave};
    envelope envelope {wave};
    phaser   phaser {wave};
    noise    noise {wave};
    if (wave.WaveType == sound_wave::type::Noise) { noise.generate(); }

    std::vector<f32> samples(static_cast<usize>(MAX_WAVE_LENGTH_SECONDS * wave.SampleRate));

    bool generatingSample {true};
    i32  sampleCount {0};
    i32  phase {0};

    for (i32 i {0}; i < MAX_WAVE_LENGTH_SECONDS * wave.SampleRate; ++i) {
        if (!generatingSample) {
            sampleCount = i;
            break;
        }

        if (repeatLimit != 0 && ++repeatTime >= repeatLimit) { reset(); }

        // Volume envelope
        if (!envelope.step()) { generatingSample = false; }

        phaser.step();
        filter.step();

        auto const fperiod {wavePeriod()};
        if (wavePeriod.FrequencyOutOfBounds) { generatingSample = false; }
        i32 const period {std::max(8, static_cast<i32>(vibrato(fperiod)))};
        f32       ssample {0.0f};

        // Supersampling x8
        for (i32 si {0}; si < MAX_SUPERSAMPLING; ++si) {
            ++phase;

            if (phase >= period) {
                phase %= period;
                if (wave.WaveType == sound_wave::type::Noise) { noise.generate(); }
            }

            // base waveform
            f32 const fp {static_cast<f32>(phase) / static_cast<f32>(period)};

            f32 sample {0.0f};

            switch (wave.WaveType) {
            case sound_wave::type::Square:   sample = fp < squareDuty() ? 0.5f : -0.5f; break;
            case sound_wave::type::Sawtooth: sample = 1.0f - (fp * 2); break;
            case sound_wave::type::Sine:     sample = std::sin(fp * TAU_F); break;
            case sound_wave::type::Noise:    sample = noise[phase * 32 / period]; break;
            case sound_wave::type::Triangle: sample = 1.0f - (std::abs(std::round(fp) - fp) * 4); break;
            }

            sample = filter(sample);
            sample = phaser(sample);

            // Final accumulation and envelope application
            ssample += sample * envelope();
        }

        ssample = (ssample / MAX_SUPERSAMPLING) * SAMPLE_SCALE_COEFICIENT;

        // Accumulate samples in the buffer
        samples[static_cast<usize>(i)] = std::clamp(ssample, -1.0f, 1.0f);
    }

    samples.resize(static_cast<usize>(sampleCount));
    return buffer::Create({.Channels = 1, .SampleRate = wave.SampleRate}, samples);
}

////////////////////////////////////////////////////////////

void sound_wave::sanitize()
{
    AttackTime   = std::clamp(AttackTime, 0.0f, 1.0f);
    SustainTime  = std::clamp(SustainTime, 0.0f, 1.0f);
    SustainPunch = std::clamp(SustainPunch, 0.0f, 1.0f);
    DecayTime    = std::clamp(DecayTime, 0.0f, 1.0f);

    MinFrequency   = std::clamp(MinFrequency, 0.0f, 1.0f);
    StartFrequency = std::clamp(std::max(MinFrequency, StartFrequency), 0.0f, 1.0f);
    DeltaSlide     = std::clamp(DeltaSlide, -1.0f, 1.0f);
    Slide          = std::clamp(Slide, -1.0f, 1.0f);
    VibratoDepth   = std::clamp(VibratoDepth, 0.0f, 1.0f);
    VibratoSpeed   = std::clamp(VibratoSpeed, 0.0f, 1.0f);

    ChangeAmount = std::clamp(ChangeAmount, -1.0f, 1.0f);
    ChangeSpeed  = std::clamp(ChangeSpeed, 0.0f, 1.0f);

    SquareDuty = std::clamp(SquareDuty, 0.0f, 1.0f);
    DutySweep  = std::clamp(DutySweep, -1.0f, 1.0f);

    RepeatSpeed = std::clamp(RepeatSpeed, 0.0f, 1.0f);

    PhaserOffset = std::clamp(PhaserOffset, -1.0f, 1.0f);
    PhaserSweep  = std::clamp(PhaserSweep, -1.0f, 1.0f);

    LowPassFilterCutoff       = std::clamp(LowPassFilterCutoff, 0.0f, 1.0f);
    LowPassFilterCutoffSweep  = std::clamp(LowPassFilterCutoffSweep, -1.0f, 1.0f);
    LowPassFilterResonance    = std::clamp(LowPassFilterResonance, 0.0f, 1.0f);
    HighPassFilterCutoff      = std::clamp(HighPassFilterCutoff, 0.0f, 1.0f);
    HighPassFilterCutoffSweep = std::clamp(HighPassFilterCutoffSweep, -1.0f, 1.0f);
}
}
