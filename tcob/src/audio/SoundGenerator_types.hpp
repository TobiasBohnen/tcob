// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

// based on: sfxr and rFXGen

#include "tcob/audio/SoundGenerator.hpp"

namespace tcob::audio {

////////////////////////////////////////////////////////////

class filter {
public:
    explicit filter(sound_wave const& wave)
        : _fltw {std::pow(wave.LowPassFilterCutoff, 3.0f) * 0.1f}
        , _fltwd {1.0f + (wave.LowPassFilterCutoffSweep * 0.0001f)}
        , _fltdmp {std::min(0.8f, 5.0f / (1.0f + std::pow(wave.LowPassFilterResonance, 2.0f) * 20.0f) * (0.01f + _fltw))}
        , _flthp {std::pow(wave.HighPassFilterCutoff, 2.0f) * 0.1f}
        , _flthpd {1.0f + (wave.HighPassFilterCutoffSweep * 0.0003f)}
        , _lpfcCheck {wave.LowPassFilterCutoff != 1.0f}
    {
    }

    void step()
    {
        if (_flthpd != 0.0f) {
            _flthp *= _flthpd;
            _flthp = std::clamp(_flthp, 0.00001f, 0.1f);
        }
    }

    void apply(f32& sample)
    {
        // LP filter
        f32 const pp {_fltp};
        _fltw *= _fltwd;

        _fltw = std::clamp(_fltw, 0.0f, 0.1f);

        if (_lpfcCheck) {
            _fltdp += (sample - _fltp) * _fltw;
            _fltdp -= _fltdp * _fltdmp;
        } else {
            _fltp  = sample;
            _fltdp = 0.0f;
        }

        _fltp += _fltdp;

        // HP filter
        _fltphp += _fltp - pp;
        _fltphp -= _fltphp * _flthp;
        sample = _fltphp;
    }

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
    explicit envelope(sound_wave const& wave)
        : _attackTime {static_cast<i32>(wave.AttackTime * wave.AttackTime * 100000.0f)}
        , _sustainTime {static_cast<i32>(wave.SustainTime * wave.SustainTime * 100000.0f)}
        , _decayTime {static_cast<i32>(wave.DecayTime * wave.DecayTime * 100000.0f)}
        , _sustainPunch {wave.SustainPunch}
    {
    }

    auto increment_time() -> bool
    {
        ++_time;
        while (_time > get_stage_time()) {
            _time = 0;
            ++_stage;

            if (_stage >= 3) { return false; }
        }

        return true;
    }

    auto get() const -> f32
    {
        switch (_stage) {
        case 0: return static_cast<f32>(_time) / static_cast<f32>(_attackTime);
        case 1: return 1.0f + (std::pow(1.0f - (static_cast<f32>(_time) / static_cast<f32>(_sustainTime)), 1.0f) * 2.0f * _sustainPunch);
        case 2: return 1.0f - (static_cast<f32>(_time) / static_cast<f32>(_decayTime));
        }

        return 0;
    }

private:
    auto get_stage_time() const -> i32
    {
        switch (_stage) {
        case 0: return _attackTime;
        case 1: return _sustainTime;
        case 2: return _decayTime;
        }

        return 0;
    }

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
    explicit phaser(sound_wave const& wave)
        : _fphase {std::pow(wave.PhaserOffset, 2.0f) * 1020.0f}
        , _fdphase {std::pow(wave.PhaserSweep, 2.0f) * 1.0f}
    {
        if (wave.PhaserOffset < 0.0f) { _fphase = -_fphase; }
        if (wave.PhaserSweep < 0.0f) { _fdphase = -_fdphase; }
    }

    void step()
    {
        _fphase += _fdphase;
        _iphase = std::min(1023, std::abs(static_cast<i32>(_fphase)));
    }

    void apply(f32& sample)
    {
        _phaserBuffer[_ipp & 1023] = sample;
        sample += _phaserBuffer[(_ipp - _iphase + 1024) & 1023];
        _ipp = (_ipp + 1) & 1023;
    }

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
    explicit noise(sound_wave const& wave)
        : _random {wave.RandomSeed}
    {
    }

    void generate()
    {
        for (f32& f : _buffer) {
            f = _random(-1.0f, 1.0f);
        }
    }

    auto get(i32 idx) const -> f32
    {
        return _buffer[idx];
    }

private:
    random::rng_split_mix_64 _random {};
    std::array<f32, 32>      _buffer {};
};

////////////////////////////////////////////////////////////

class vibrato {
public:
    explicit vibrato(sound_wave const& wave)
        : _speed {std::pow(wave.VibratoSpeed, 2.0f) * 0.01f}
        , _amplitude {wave.VibratoDepth * 0.5f}
    {
    }

    auto get(f64 fperiod) -> f32
    {
        if (_amplitude > 0.0f) {
            _phase += _speed;
            return static_cast<f32>(fperiod * (1.0f + std::sin(_phase) * _amplitude));
        }

        return static_cast<f32>(fperiod);
    }

private:
    f32 _speed;
    f32 _amplitude;
    f32 _phase {0.0f};
};

////////////////////////////////////////////////////////////

class arpeggio {
public:
    void reset(sound_wave const& wave)
    {
        _modulation =
            wave.ChangeAmount >= 0.0f
            ? 1.0 - (std::pow(static_cast<f64>(wave.ChangeAmount), 2.0) * 0.9)
            : 1.0 + (std::pow(static_cast<f64>(wave.ChangeAmount), 2.0) * 10.0);

        _limit = static_cast<i32>((std::pow(1.0f - wave.ChangeSpeed, 2.0f) * 20000) + 32);

        _time = 0;
    }

    void apply(f64& fperiod)
    {
        ++_time;

        if (_limit != 0 && _time >= _limit) {
            _limit = 0;
            fperiod *= _modulation;
        }
    }

private:
    f64 _modulation {0};
    i32 _limit {0};
    i32 _time {0};
};

}
