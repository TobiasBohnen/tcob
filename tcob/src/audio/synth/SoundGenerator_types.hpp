// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

// based on: sfxr and rFXGen

#include "tcob/audio/synth/SoundGenerator.hpp"

namespace tcob::audio {

////////////////////////////////////////////////////////////

class filter {
public:
    explicit filter(sound_wave const& wave);

    void step();

    auto operator()(f32 sample) -> f32;

private:
    f32       _fltw;
    f32 const _fltwd;
    f32 const _fltdmp;
    f32       _flthp;
    f32       _flthpd;

    f32 _fltp {0.0f};
    f32 _fltdp {0.0f};
    f32 _fltphp {0.0f};

    bool _lpfcCheck;
};

////////////////////////////////////////////////////////////

class envelope {
public:
    explicit envelope(sound_wave const& wave);

    auto step() -> bool;

    auto operator()() const -> f32;

private:
    auto get_stage_time() const -> i32;

    i32 _attackTime {0};
    i32 _sustainTime {0};
    i32 _decayTime {0};

    i32 _stage {0};
    i32 _time {0};

    f32 _sustainPunch {0};
};

////////////////////////////////////////////////////////////

class phaser {
public:
    explicit phaser(sound_wave const& wave);

    void step();

    auto operator()(f32 sample) -> f32;

private:
    f32                   _fphase {0};
    f32                   _fdphase {0};
    i32                   _iphase {0};
    i32                   _ipp {0};
    std::array<f32, 1024> _phaserBuffer {};
};

////////////////////////////////////////////////////////////

class noise {
public:
    explicit noise(sound_wave const& wave);

    void generate();

    auto operator[](i32 idx) const -> f32;

private:
    random::rng_split_mix_64 _random {};
    std::array<f32, 32>      _buffer {};
};

////////////////////////////////////////////////////////////

class vibrato {
public:
    explicit vibrato(sound_wave const& wave);

    auto operator()(f64 fperiod) -> f32;

private:
    f32 _speed;
    f32 _amplitude;
    f32 _phase {0.0f};
};

////////////////////////////////////////////////////////////

class arpeggio {
public:
    arpeggio();
    arpeggio(sound_wave const& wave);

    auto operator()(f64 fperiod) -> f64;

private:
    f64 _modulation {0};
    i32 _limit {0};
    i32 _time {0};
};

////////////////////////////////////////////////////////////

class square_duty {
public:
    square_duty() = default;
    square_duty(sound_wave const& wave);

    auto operator()() -> f32;

private:
    f32 _squareDuty {0};
    f32 _squareSlide {0};
};

////////////////////////////////////////////////////////////

class period {
public:
    period() = default;
    period(sound_wave const& wave);

    auto operator()() -> f64;

    bool FrequencyOutOfBounds {false};

private:
    f64 _period {};
    f64 _maxperiod {};
    f64 _slide {};
    f64 _deltaSlide {};

    arpeggio _arpeggio {};
};

}
