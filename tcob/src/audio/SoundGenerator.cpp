// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on: sfxr and rFXGen

#include "tcob/audio/SoundGenerator.hpp"

#include "ALObjects.hpp"

namespace tcob::audio {

void static generate_noise(std::span<f32> buffer, random::rng_split_mix_64& random)
{
    for (f32& f : buffer) {
        f = random(-1.0f, 1.0f);
    }
}

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

    return sanitize_wave(retValue);
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

    return sanitize_wave(retValue);
}

auto sound_generator::sanitize_wave(sound_wave const& wave) -> sound_wave
{
    return {
        .RandomSeed = wave.RandomSeed,
        .WaveType   = wave.WaveType,

        .AttackTime   = std::clamp(wave.AttackTime, 0.0f, 1.0f),
        .SustainTime  = std::clamp(wave.SustainTime, 0.0f, 1.0f),
        .SustainPunch = std::clamp(wave.SustainPunch, 0.0f, 1.0f),
        .DecayTime    = std::clamp(wave.DecayTime, 0.0f, 1.0f),

        .StartFrequency = std::clamp(wave.StartFrequency, 0.0f, 1.0f),
        .MinFrequency   = std::clamp(wave.MinFrequency, 0.0f, 1.0f),
        .Slide          = std::clamp(wave.Slide, -1.0f, 1.0f),
        .DeltaSlide     = std::clamp(wave.DeltaSlide, -1.0f, 1.0f),
        .VibratoDepth   = std::clamp(wave.VibratoDepth, 0.0f, 1.0f),
        .VibratoSpeed   = std::clamp(wave.VibratoSpeed, 0.0f, 1.0f),

        .ChangeAmount = std::clamp(wave.ChangeAmount, -1.0f, 1.0f),
        .ChangeSpeed  = std::clamp(wave.ChangeSpeed, 0.0f, 1.0f),

        .SquareDuty = std::clamp(wave.SquareDuty, 0.0f, 1.0f),
        .DutySweep  = std::clamp(wave.DutySweep, -1.0f, 1.0f),

        .RepeatSpeed = std::clamp(wave.RepeatSpeed, 0.0f, 1.0f),

        .PhaserOffset = std::clamp(wave.PhaserOffset, -1.0f, 1.0f),
        .PhaserSweep  = std::clamp(wave.PhaserSweep, -1.0f, 1.0f),

        .LowPassFilterCutoff       = std::clamp(wave.LowPassFilterCutoff, 0.0f, 1.0f),
        .LowPassFilterCutoffSweep  = std::clamp(wave.LowPassFilterCutoffSweep, -1.0f, 1.0f),
        .LowPassFilterResonance    = std::clamp(wave.LowPassFilterResonance, 0.0f, 1.0f),
        .HighPassFilterCutoff      = std::clamp(wave.HighPassFilterCutoff, 0.0f, 1.0f),
        .HighPassFilterCutoffSweep = std::clamp(wave.HighPassFilterCutoffSweep, -1.0f, 1.0f)};
}

// Generates new wave from wave parameters
// NOTE: By default wave is generated as 44100Hz, 32bit float, mono
auto sound_generator::create_buffer(sound_wave const& wave) -> buffer
{
    random::rng_split_mix_64 noiseRandom {wave.RandomSeed};

    sound_wave    newWave {wave};
    constexpr i32 MAX_WAVE_LENGTH_SECONDS {10};   // Max length for wave: 10 seconds
    constexpr i32 MAX_SUPERSAMPLING {8};
    constexpr f32 SAMPLE_SCALE_COEFICIENT {0.2f}; // NOTE: Used to scale sample value to [-1..1]

    // Configuration parameters for generation
    // NOTE: Those parameters are calculated from selected values

    if (newWave.MinFrequency > newWave.StartFrequency) {
        newWave.MinFrequency = newWave.StartFrequency;
    }
    if (newWave.Slide < newWave.DeltaSlide) {
        newWave.Slide = newWave.DeltaSlide;
    }

    // Reset sample parameters
    //----------------------------------------------------------------------------------------
    f64 fperiod {100.0 / (newWave.StartFrequency * newWave.StartFrequency + 0.001)};
    f64 fmaxperiod {100.0 / (newWave.MinFrequency * newWave.MinFrequency + 0.001)};
    f64 fslide {1.0 - std::pow(static_cast<f64>(newWave.Slide), 3.0) * 0.01};
    f64 fdslide {-std::pow(static_cast<f64>(newWave.DeltaSlide), 3.0) * 0.000001};
    f32 squareDuty {0.5f - newWave.SquareDuty * 0.5f};
    f32 squareSlide {-newWave.DutySweep * 0.00005f};

    f64 arpeggioModulation {
        newWave.ChangeAmount >= 0.0f
            ? 1.0 - std::pow(static_cast<f64>(newWave.ChangeAmount), 2.0) * 0.9
            : 1.0 + std::pow(static_cast<f64>(newWave.ChangeAmount), 2.0) * 10.0};

    i32 arpeggioLimit {static_cast<i32>(std::pow(1.0f - newWave.ChangeSpeed, 2.0f) * 20000 + 32)};
    if (newWave.ChangeSpeed == 1.0f) {
        arpeggioLimit = 0; // WATCH OUT: f32 comparison
    }

    // Reset filter parameters
    f32 fltw {std::pow(newWave.LowPassFilterCutoff, 3.0f) * 0.1f};
    f32 fltwd {1.0f + newWave.LowPassFilterCutoffSweep * 0.0001f};
    f32 fltdmp {5.0f / (1.0f + std::pow(newWave.LowPassFilterResonance, 2.0f) * 20.0f) * (0.01f + fltw)};
    if (fltdmp > 0.8f) {
        fltdmp = 0.8f;
    }
    f32 flthp {std::pow(newWave.HighPassFilterCutoff, 2.0f) * 0.1f};
    f32 flthpd {1.0f + newWave.HighPassFilterCutoffSweep * 0.0003f};

    // Reset vibrato
    f32 vibratoSpeed {std::pow(newWave.VibratoSpeed, 2.0f) * 0.01f};
    f32 vibratoAmplitude {newWave.VibratoDepth * 0.5f};

    // Reset envelope
    std::array<i32, 3> envelopeLength {
        static_cast<i32>(newWave.AttackTime * newWave.AttackTime * 100000.0f),
        static_cast<i32>(newWave.SustainTime * newWave.SustainTime * 100000.0f),
        static_cast<i32>(newWave.DecayTime * newWave.DecayTime * 100000.0f)};

    f32 fphase {std::pow(newWave.PhaserOffset, 2.0f) * 1020.0f};
    if (newWave.PhaserOffset < 0.0f) {
        fphase = -fphase;
    }

    f32 fdphase {std::pow(newWave.PhaserSweep, 2.0f) * 1.0f};
    if (newWave.PhaserSweep < 0.0f) {
        fdphase = -fdphase;
    }

    i32 iphase {0};

    std::array<f32, 32> noiseBuffer {}; // Required for noise wave, depends on random seed!
    if (newWave.WaveType == sound_wave::type::Noise) {
        generate_noise(noiseBuffer, noiseRandom);
    }

    i32 repeatLimit {newWave.RepeatSpeed == 0.0f
                         ? 0
                         : static_cast<i32>(std::pow(1.0f - newWave.RepeatSpeed, 2.0f) * 20000 + 32)};

    //----------------------------------------------------------------------------------------

    // NOTE: We reserve enough space for up to 10 seconds of wave audio at given sample rate
    // By default we use f32 size samples, they are converted to desired sample size at the end
    std::vector<f32> samples;
    samples.resize(MAX_WAVE_LENGTH_SECONDS * wave.SampleRate);

    bool                  generatingSample {true};
    i32                   sampleCount {0};
    i32                   phase {0};
    i32                   envelopeStage {0};
    i32                   envelopeTime {0};
    f32                   envelopeVolume {0.0f};
    i32                   ipp {0};
    f32                   fltp {0.0f};
    f32                   fltdp {0.0f};
    f32                   fltphp {0.0f};
    f32                   vibratoPhase {0.0f};
    i32                   repeatTime {0};
    i32                   arpeggioTime {0};
    std::array<f32, 1024> phaserBuffer {};

    for (i32 i {0}; i < MAX_WAVE_LENGTH_SECONDS * wave.SampleRate; i++) {
        if (!generatingSample) {
            sampleCount = i;
            break;
        }

        // Generate sample using selected parameters
        //------------------------------------------------------------------------------------
        repeatTime++;

        if (repeatLimit != 0 && repeatTime >= repeatLimit) {
            // Reset sample parameters (only some of them)
            repeatTime = 0;

            fperiod     = 100.0 / (newWave.StartFrequency * newWave.StartFrequency + 0.001);
            fmaxperiod  = 100.0 / (newWave.MinFrequency * newWave.MinFrequency + 0.001);
            fslide      = 1.0 - std::pow(static_cast<f64>(newWave.Slide), 3.0) * 0.01;
            fdslide     = -std::pow(static_cast<f64>(newWave.DeltaSlide), 3.0) * 0.000001;
            squareDuty  = 0.5f - newWave.SquareDuty * 0.5f;
            squareSlide = -newWave.DutySweep * 0.00005f;

            arpeggioModulation = newWave.ChangeAmount >= 0.0f
                ? 1.0 - std::pow(static_cast<f64>(newWave.ChangeAmount), 2.0) * 0.9
                : 1.0 + std::pow(static_cast<f64>(newWave.ChangeAmount), 2.0) * 10.0;

            arpeggioTime  = 0;
            arpeggioLimit = newWave.ChangeSpeed == 1.0f
                ? 0
                : static_cast<i32>(std::pow(1.0f - newWave.ChangeSpeed, 2.0f) * 20000 + 32);
        }

        // Frequency envelopes/arpeggios
        arpeggioTime++;

        if (arpeggioLimit != 0 && arpeggioTime >= arpeggioLimit) {
            arpeggioLimit = 0;
            fperiod *= arpeggioModulation;
        }

        fslide += fdslide;
        fperiod *= fslide;

        if (fperiod > fmaxperiod) {
            fperiod = fmaxperiod;

            if (newWave.MinFrequency > 0.0f) {
                generatingSample = false;
            }
        }

        f32 rfperiod {static_cast<f32>(fperiod)};

        if (vibratoAmplitude > 0.0f) {
            vibratoPhase += vibratoSpeed;
            rfperiod = static_cast<f32>(fperiod * (1.0f + std::sin(vibratoPhase) * vibratoAmplitude));
        }

        i32 const period {std::max(8, static_cast<i32>(rfperiod))};

        squareDuty += squareSlide;
        squareDuty = std::clamp(squareDuty, 0.0f, 0.5f);

        // Volume envelope
        envelopeTime++;

        while (envelopeTime > envelopeLength[envelopeStage]) {
            envelopeTime = 0;
            envelopeStage++;

            if (envelopeStage == 3) {
                generatingSample = false;
                break;
            }
        }

        switch (envelopeStage) {
        case 0:
            envelopeVolume = static_cast<f32>(envelopeTime) / static_cast<f32>(envelopeLength[0]);
            break;
        case 1:
            envelopeVolume = 1.0f + std::pow(1.0f - static_cast<f32>(envelopeTime) / static_cast<f32>(envelopeLength[1]), 1.0f) * 2.0f * newWave.SustainPunch;
            break;
        case 2:
            envelopeVolume = 1.0f - static_cast<f32>(envelopeTime) / static_cast<f32>(envelopeLength[2]);
            break;
        default:
            break;
        }

        // Phaser step
        fphase += fdphase;
        iphase = std::min(1023, std::abs(static_cast<i32>(fphase)));

        if (flthpd != 0.0f) {
            flthp *= flthpd;
            flthp = std::clamp(flthp, 0.00001f, 0.1f);
        }

        f32 ssample {0.0f};

        // Supersampling x8
        for (i32 si {0}; si < MAX_SUPERSAMPLING; ++si) {
            f32 sample {0.0f};
            phase++;

            if (phase >= period) {
                phase %= period;

                if (newWave.WaveType == sound_wave::type::Noise) {
                    generate_noise(noiseBuffer, noiseRandom);
                }
            }

            // base waveform
            f32 const fp {static_cast<f32>(phase) / static_cast<f32>(period)};

            switch (newWave.WaveType) {
            case sound_wave::type::Square:
                sample = fp < squareDuty ? 0.5f : -0.5f;
                break;
            case sound_wave::type::Sawtooth:
                sample = 1.0f - fp * 2;
                break;
            case sound_wave::type::Sine:
                sample = std::sin(fp * TAU_F);
                break;
            case sound_wave::type::Noise:
                sample = noiseBuffer[phase * 32 / period];
                break;
            case sound_wave::type::Triangle:
                sample = 1.0f - std::abs(std::round(fp) - fp) * 4;
                break;
            }

            // LP filter
            f32 const pp {fltp};
            fltw *= fltwd;

            fltw = std::clamp(fltw, 0.0f, 0.1f);

            if (newWave.LowPassFilterCutoff != 1.0f) // WATCH OUT!
            {
                fltdp += (sample - fltp) * fltw;
                fltdp -= fltdp * fltdmp;
            } else {
                fltp  = sample;
                fltdp = 0.0f;
            }

            fltp += fltdp;

            // HP filter
            fltphp += fltp - pp;
            fltphp -= fltphp * flthp;
            sample = fltphp;

            // Phaser
            phaserBuffer[ipp & 1023] = sample;
            sample += phaserBuffer[(ipp - iphase + 1024) & 1023];
            ipp = (ipp + 1) & 1023;

            // Final accumulation and envelope application
            ssample += sample * envelopeVolume;
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

}
