// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on: sfxr and rFXGen

#include "tcob/audio/SoundGenerator.hpp"

#include "ALObjects.hpp"
#include "SoundGenerator_types.hpp"

namespace tcob::audio {

////////////////////////////////////////////////////////////

sound_generator::sound_generator(random::rng_split_mix_64 random)
    : _random {random}
{
}

// Generate sound: Pickup/Coin
auto sound_generator::generate_pickup_coin() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    retValue.StartFrequency = _random(0.4f, 0.9f);
    retValue.AttackTime     = 0.0f;
    retValue.SustainTime    = _random(0.0f, 0.1f);
    retValue.DecayTime      = _random(0.1f, 0.5f);
    retValue.SustainPunch   = _random(0.3f, 0.6f);

    if (_random(0, 1) == 1) {
        retValue.ChangeSpeed  = _random(0.5f, 0.7f);
        retValue.ChangeAmount = _random(0.2f, 0.6f);
    }

    return retValue;
}

// Generate sound: Laser shoot
auto sound_generator::generate_laser_shoot() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    retValue.WaveType = static_cast<sound_wave::type>(_random(0, 2));

    if ((retValue.WaveType == sound_wave::type::Sine) && _random(0, 1) == 1) {
        retValue.WaveType = static_cast<sound_wave::type>(_random(0, 1));
    }

    retValue.StartFrequency = _random(0.5f, 1.0f);
    retValue.MinFrequency   = retValue.StartFrequency - _random(0.2f, 0.8f);

    if (retValue.MinFrequency < 0.2f) {
        retValue.MinFrequency = 0.2f;
    }

    retValue.Slide = -_random(-0.35f, -0.15f);

    if (_random(0, 2) == 0) {
        retValue.StartFrequency = _random(0.3f, 0.9f);
        retValue.MinFrequency   = _random(0.0f, 0.1f);
        retValue.Slide          = _random(-0.35f, -0.05f);
    }

    if (_random(0, 1) == 1) {
        retValue.SquareDuty = _random(0.0f, 0.5f);
        retValue.DutySweep  = _random(0.0f, 0.2f);
    } else {
        retValue.SquareDuty = _random(0.4f, 0.9f);
        retValue.DutySweep  = -_random(0.0f, 0.7f);
    }

    retValue.AttackTime  = 0.0f;
    retValue.SustainTime = _random(0.1f, 0.3f);
    retValue.DecayTime   = _random(0.0f, 0.4f);

    if (_random(0, 1) == 1) {
        retValue.SustainPunch = _random(0.0f, 0.3f);
    }

    if (_random(0, 2) == 0) {
        retValue.PhaserOffset = _random(0.0f, 0.2f);
        retValue.PhaserSweep  = -_random(0.0f, 0.2f);
    }

    if (_random(0, 1) == 1) {
        retValue.HighPassFilterCutoff = _random(0.0f, 0.3f);
    }

    return retValue;
}

// Generate sound: Explosion
auto sound_generator::generate_explosion() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    retValue.WaveType = sound_wave::type::Noise;

    if (_random(0, 1) == 1) {
        retValue.StartFrequency = _random(0.1f, 0.5f);
        retValue.Slide          = _random(-0.1f, 0.3f);
    } else {
        retValue.StartFrequency = _random(0.2f, 0.7f);
        retValue.Slide          = _random(-0.4f, -0.2f);
    }

    retValue.StartFrequency *= retValue.StartFrequency;

    if (_random(0, 4) == 0) {
        retValue.Slide = 0.0f;
    }
    if (_random(0, 2) == 0) {
        retValue.RepeatSpeed = _random(0.3f, 0.8f);
    }

    retValue.AttackTime  = 0.0f;
    retValue.SustainTime = _random(0.1f, 0.4f);
    retValue.DecayTime   = _random(0.0f, 0.5f);

    if (_random(0, 1) == 1) {
        retValue.PhaserOffset = _random(-0.3f, 0.6f);
        retValue.PhaserSweep  = -_random(0.0f, 0.3f);
    }

    retValue.SustainPunch = _random(0.2f, 0.8f);

    if (_random(0, 1) == 1) {
        retValue.VibratoDepth = _random(0.0f, 0.7f);
        retValue.VibratoSpeed = _random(0.0f, 0.6f);
    }

    if (_random(0, 2) == 0) {
        retValue.ChangeSpeed  = _random(0.6f, 0.9f);
        retValue.ChangeAmount = _random(-0.8f, 0.8f);
    }

    return retValue;
}

// Generate sound: Powerup
auto sound_generator::generate_powerup() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    if (_random(0, 1) == 1) {
        retValue.WaveType = sound_wave::type::Sawtooth;
    } else {
        retValue.SquareDuty = _random(0.0f, 0.6f);
    }

    if (_random(0, 1) == 1) {
        retValue.StartFrequency = _random(0.2f, 0.5f);
        retValue.Slide          = _random(0.1f, 0.5f);
        retValue.RepeatSpeed    = _random(0.4f, 0.8f);
    } else {
        retValue.StartFrequency = _random(0.5f, 0.5f);
        retValue.Slide          = _random(0.05f, 0.205f);

        if (_random(0, 1) == 1) {
            retValue.VibratoDepth = _random(0.0f, 0.7f);
            retValue.VibratoSpeed = _random(0.0f, 0.6f);
        }
    }

    retValue.AttackTime  = 0.0f;
    retValue.SustainTime = _random(0.0f, 0.4f);
    retValue.DecayTime   = _random(0.1f, 0.5f);

    return retValue;
}

// Generate sound: Hit/Hurt
auto sound_generator::generate_hit_hurt() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    retValue.WaveType = static_cast<sound_wave::type>(_random(0, 2));
    if (retValue.WaveType == sound_wave::type::Sine) {
        retValue.WaveType = sound_wave::type::Noise;
    }
    if (retValue.WaveType == sound_wave::type::Square) {
        retValue.SquareDuty = _random(0.0f, 0.6f);
    }

    retValue.StartFrequency = _random(0.2f, 0.8f);
    retValue.Slide          = _random(-0.7f, -0.3f);
    retValue.AttackTime     = 0.0f;
    retValue.SustainTime    = _random(0.0f, 0.1f);
    retValue.DecayTime      = _random(0.1f, 0.3f);

    if (_random(0, 1) == 1) {
        retValue.HighPassFilterCutoff = _random(0.0f, 0.3f);
    }

    return retValue;
}

// Generate sound: Jump
auto sound_generator::generate_jump() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    retValue.WaveType       = sound_wave::type::Square;
    retValue.SquareDuty     = _random(0.0f, 0.6f);
    retValue.StartFrequency = _random(0.3f, 0.6f);
    retValue.Slide          = _random(0.1f, 0.3f);
    retValue.AttackTime     = 0.0f;
    retValue.SustainTime    = _random(0.1f, 0.4f);
    retValue.DecayTime      = _random(0.1f, 0.3f);

    if (_random(0, 1) == 1) {
        retValue.HighPassFilterCutoff = _random(0.0f, 0.3f);
    }
    if (_random(0, 1) == 1) {
        retValue.LowPassFilterCutoff = _random(0.4f, 1.0f);
    }

    return retValue;
}

// Generate sound: Blip/Select
auto sound_generator::generate_blip_select() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    retValue.WaveType = static_cast<sound_wave::type>(_random(0, 1));
    if (retValue.WaveType == sound_wave::type::Square) {
        retValue.SquareDuty = _random(0.0f, 0.6f);
    }
    retValue.StartFrequency       = _random(0.2f, 0.6f);
    retValue.AttackTime           = 0.0f;
    retValue.SustainTime          = _random(0.1f, 0.2f);
    retValue.DecayTime            = _random(0.0f, 0.2f);
    retValue.HighPassFilterCutoff = 0.1f;

    return retValue;
}

// Generate random sound
auto sound_generator::generate_random() -> sound_wave
{
    sound_wave retValue {};
    retValue.RandomSeed = _random.get_state()[0];

    retValue.StartFrequency = std::pow(_random(-1.0f, 1.0f), 2.0f);

    if (_random(0, 1) == 1) {
        retValue.StartFrequency = std::pow(_random(-1.0f, 1.0f), 3.0f) + 0.5f;
    }

    retValue.MinFrequency = 0.0f;
    retValue.Slide        = std::pow(_random(-1.0f, 1.0f), 5.0f);

    if ((retValue.StartFrequency > 0.7f) && (retValue.Slide > 0.2f)) {
        retValue.Slide = -retValue.Slide;
    }
    if ((retValue.StartFrequency < 0.2f) && (retValue.Slide < -0.05f)) {
        retValue.Slide = -retValue.Slide;
    }

    retValue.DeltaSlide   = std::pow(_random(-1.0f, 1.0f), 3.0f);
    retValue.SquareDuty   = _random(-1.0f, 1.0f);
    retValue.DutySweep    = std::pow(_random(-1.0f, 1.0f), 3.0f);
    retValue.VibratoDepth = std::pow(_random(-1.0f, 1.0f), 3.0f);
    retValue.VibratoSpeed = _random(-1.0f, 1.0f);
    // retValue.vibratoPhaseDelay = _random(-1.0f, 1.0f);
    retValue.AttackTime   = std::pow(_random(-1.0f, 1.0f), 3.0f);
    retValue.SustainTime  = std::pow(_random(-1.0f, 1.0f), 2.0f);
    retValue.DecayTime    = _random(-1.0f, 1.0f);
    retValue.SustainPunch = std::pow(_random(0.0f, 0.8f), 2.0f);

    if (retValue.AttackTime + retValue.SustainTime + retValue.DecayTime < 0.2f) {
        retValue.SustainTime += 0.2f + _random(0.0f, 0.3f);
        retValue.DecayTime += 0.2f + _random(0.0f, 0.3f);
    }

    retValue.LowPassFilterResonance   = _random(-1.0f, 1.0f);
    retValue.LowPassFilterCutoff      = 1.0f - std::pow(_random(0.0f, 1.0f), 3.0f);
    retValue.LowPassFilterCutoffSweep = std::pow(_random(-1.0f, 1.0f), 3.0f);

    if (retValue.LowPassFilterCutoff < 0.1f && retValue.LowPassFilterCutoffSweep < -0.05f) {
        retValue.LowPassFilterCutoffSweep = -retValue.LowPassFilterCutoffSweep;
    }

    retValue.HighPassFilterCutoff      = std::pow(_random(0.0f, 1.0f), 5.0f);
    retValue.HighPassFilterCutoffSweep = std::pow(_random(-1.0f, 1.0f), 5.0f);
    retValue.PhaserOffset              = std::pow(_random(-1.0f, 1.0f), 3.0f);
    retValue.PhaserSweep               = std::pow(_random(-1.0f, 1.0f), 3.0f);
    retValue.RepeatSpeed               = _random(-1.0f, 1.0f);
    retValue.ChangeSpeed               = _random(-1.0f, 1.0f);
    retValue.ChangeAmount              = _random(-1.0f, 1.0f);

    retValue.sanitize();
    return retValue;
}

// Mutate current sound
auto sound_generator::mutate_wave(sound_wave const& wave) -> sound_wave
{
    sound_wave retValue {wave};

    if (_random(0, 1) == 1) { retValue.StartFrequency += _random(-0.05f, 0.05f); }
    // if (_random(0, 1) == 1) retValue.minFrequency += _random(-0.05f, 0.05f);
    if (_random(0, 1) == 1) { retValue.Slide += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.DeltaSlide += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.SquareDuty += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.DutySweep += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.VibratoDepth += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.VibratoSpeed += _random(-0.05f, 0.05f); }
    // if (_random(0, 1) == 1) wave.vibratoPhaseDelay += _random(-0.05f, 0.05f);
    if (_random(0, 1) == 1) { retValue.AttackTime += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.SustainTime += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.DecayTime += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.SustainPunch += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.LowPassFilterResonance += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.LowPassFilterCutoff += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.LowPassFilterCutoffSweep += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.HighPassFilterCutoff += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.HighPassFilterCutoffSweep += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.PhaserOffset += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.PhaserSweep += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.RepeatSpeed += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.ChangeSpeed += _random(-0.05f, 0.05f); }
    if (_random(0, 1) == 1) { retValue.ChangeAmount += _random(-0.05f, 0.05f); }

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

    i32 repeatTime {0};

    f64 fperiod {0};
    f64 fmaxperiod {0};
    f64 fslide {0};
    f64 fdslide {0};
    f32 squareDuty {0};
    f32 squareSlide {0};

    arpeggio arpeggio;

    auto const reset_sample_parameters {[&]() {
        repeatTime = 0;

        fperiod    = {100.0 / (wave.StartFrequency * wave.StartFrequency + 0.001)};
        fmaxperiod = {100.0 / (wave.MinFrequency * wave.MinFrequency + 0.001)};

        fslide  = {1.0 - std::pow(static_cast<f64>(wave.Slide), 3.0) * 0.01};
        fdslide = {-std::pow(static_cast<f64>(wave.DeltaSlide), 3.0) * 0.000001};

        squareDuty  = {0.5f - wave.SquareDuty * 0.5f};
        squareSlide = {-wave.DutySweep * 0.00005f};

        arpeggio.reset(wave);
    }};

    reset_sample_parameters();

    // filter
    filter filter {wave};

    // vibrato
    vibrato vibrato {wave};

    // envelope
    envelope envelope {wave};

    // phaser
    phaser phaser {wave};

    // noise
    noise noise {wave};
    if (wave.WaveType == sound_wave::type::Noise) { noise.generate(); }

    i32 repeatLimit {wave.RepeatSpeed == 0.0f
                         ? 0
                         : static_cast<i32>(std::pow(1.0f - wave.RepeatSpeed, 2.0f) * 20000 + 32)};

    // NOTE: We reserve enough space for up to 10 seconds of wave audio at given sample rate
    // By default we use f32 size samples, they are converted to desired sample size at the end
    std::vector<f32> samples;
    samples.resize(MAX_WAVE_LENGTH_SECONDS * wave.SampleRate);

    bool generatingSample {true};
    i32  sampleCount {0};
    i32  phase {0};

    for (i32 i {0}; i < MAX_WAVE_LENGTH_SECONDS * wave.SampleRate; ++i) {
        if (!generatingSample) {
            sampleCount = i;
            break;
        }

        // Generate sample using selected parameters
        ++repeatTime;

        if (repeatLimit != 0 && repeatTime >= repeatLimit) {
            // Reset sample parameters (only some of them)
            reset_sample_parameters();
        }

        // Frequency envelopes/arpeggios
        arpeggio.apply(fperiod);

        fslide += fdslide;
        fperiod *= fslide;

        if (fperiod > fmaxperiod) {
            fperiod = fmaxperiod;

            if (wave.MinFrequency > 0.0f) { generatingSample = false; }
        }

        // Volume envelope
        if (!envelope.increment_time()) { generatingSample = false; }

        // Phaser
        phaser.step();

        // Filter
        filter.step();

        i32 const period {std::max(8, static_cast<i32>(vibrato.get(fperiod)))};
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
            case sound_wave::type::Square: {
                squareDuty += squareSlide;
                squareDuty = std::clamp(squareDuty, 0.0f, 0.5f);
                sample     = fp < squareDuty ? 0.5f : -0.5f;
            } break;
            case sound_wave::type::Sawtooth:
                sample = 1.0f - fp * 2;
                break;
            case sound_wave::type::Sine:
                sample = std::sin(fp * TAU_F);
                break;
            case sound_wave::type::Noise:
                sample = noise.get(phase * 32 / period);
                break;
            case sound_wave::type::Triangle:
                sample = 1.0f - std::abs(std::round(fp) - fp) * 4;
                break;
            }

            // Filter
            filter.apply(sample);

            // Phaser
            phaser.apply(sample);

            // Final accumulation and envelope application
            ssample += sample * envelope.get();
        }

        ssample = (ssample / MAX_SUPERSAMPLING) * SAMPLE_SCALE_COEFICIENT;

        // Accumulate samples in the buffer
        samples[i] = std::clamp(ssample, -1.0f, 1.0f);
    }

    samples.resize(sampleCount);
    return buffer::Create({.Channels = 1, .SampleRate = wave.SampleRate, .FrameCount = std::ssize(samples)}, samples);
}

auto sound_generator::create_sound(sound_wave const& wave) -> sound
{
    auto audioData {create_buffer(wave)};
    return create_sound(audioData, wave);
}

auto sound_generator::create_sound [[nodiscard]] (buffer const& buffer, sound_wave const& wave) -> sound
{
    auto albuffer {std::make_shared<al::al_buffer>()};
    albuffer->buffer_data(buffer.get_data(), 1, wave.SampleRate);
    return sound {albuffer};
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
