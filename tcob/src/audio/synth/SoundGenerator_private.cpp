// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SoundGenerator_private.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "tcob/audio/synth/SoundGenerator.hpp"

namespace tcob::audio {

filter::filter(sound_wave const& wave)
    : _fltw {std::pow(wave.LowPassFilterCutoff, 3.0f) * 0.1f}
    , _fltwd {1.0f + (wave.LowPassFilterCutoffSweep * 0.0001f)}
    , _fltdmp {std::min(0.8f, 5.0f / (1.0f + std::pow(wave.LowPassFilterResonance, 2.0f) * 20.0f) * (0.01f + _fltw))}
    , _flthp {std::pow(wave.HighPassFilterCutoff, 2.0f) * 0.1f}
    , _flthpd {1.0f + (wave.HighPassFilterCutoffSweep * 0.0003f)}
    , _lpfcCheck {wave.LowPassFilterCutoff != 1.0f}
{
}

void filter::step()
{
    if (_flthpd != 0.0f) {
        _flthp *= _flthpd;
        _flthp = std::clamp(_flthp, 0.00001f, 0.1f);
    }
}

auto filter::operator()(f32 sample) -> f32
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
    return _fltphp;
}

////////////////////////////////////////////////////////////

envelope::envelope(sound_wave const& wave)
    : _attackTime {static_cast<i32>(wave.AttackTime * wave.AttackTime * 100000.0f)}
    , _sustainTime {static_cast<i32>(wave.SustainTime * wave.SustainTime * 100000.0f)}
    , _decayTime {static_cast<i32>(wave.DecayTime * wave.DecayTime * 100000.0f)}
    , _sustainPunch {wave.SustainPunch}
{
}

auto envelope::step() -> bool
{
    ++_time;
    while (_time > get_stage_time()) {
        _time = 0;
        ++_stage;

        if (_stage >= 3) { return false; }
    }

    return true;
}

auto envelope::operator()() const -> f32
{
    switch (_stage) {
    case 0: return static_cast<f32>(_time) / static_cast<f32>(_attackTime);
    case 1: return 1.0f + (std::pow(1.0f - (static_cast<f32>(_time) / static_cast<f32>(_sustainTime)), 1.0f) * 2.0f * _sustainPunch);
    case 2: return 1.0f - (static_cast<f32>(_time) / static_cast<f32>(_decayTime));
    }

    return 0;
}

auto envelope::get_stage_time() const -> i32
{
    switch (_stage) {
    case 0: return _attackTime;
    case 1: return _sustainTime;
    case 2: return _decayTime;
    }

    return 0;
}

////////////////////////////////////////////////////////////

phaser::phaser(sound_wave const& wave)
    : _fphase {std::pow(wave.PhaserOffset, 2.0f) * 1020.0f}
    , _fdphase {std::pow(wave.PhaserSweep, 2.0f) * 1.0f}
{
    if (wave.PhaserOffset < 0.0f) { _fphase = -_fphase; }
    if (wave.PhaserSweep < 0.0f) { _fdphase = -_fdphase; }
}

void phaser::step()
{
    _fphase += _fdphase;
    _iphase = std::min(1023, std::abs(static_cast<i32>(_fphase)));
}

auto phaser::operator()(f32 sample) -> f32
{
    _phaserBuffer[_ipp & 1023] = sample;
    sample += _phaserBuffer[(_ipp - _iphase + 1024) & 1023];
    _ipp = (_ipp + 1) & 1023;

    return sample;
}

////////////////////////////////////////////////////////////

noise::noise(sound_wave const& wave)
    : _random {wave.RandomSeed}
{
}

void noise::generate()
{
    for (f32& f : _buffer) {
        f = _random(-1.0f, 1.0f);
    }
}

auto noise::operator[](i32 idx) const -> f32
{
    return _buffer[static_cast<usize>(idx)];
}

////////////////////////////////////////////////////////////

vibrato::vibrato(sound_wave const& wave)
    : _speed {std::pow(wave.VibratoSpeed, 2.0f) * 0.01f}
    , _amplitude {wave.VibratoDepth * 0.5f}
{
}

auto vibrato::operator()(f64 fperiod) -> f32
{
    if (_amplitude > 0.0f) {
        _phase += _speed;
        return static_cast<f32>(fperiod * (1.0f + std::sin(_phase) * _amplitude));
    }

    return static_cast<f32>(fperiod);
}

////////////////////////////////////////////////////////////

arpeggio::arpeggio() = default;

arpeggio::arpeggio(sound_wave const& wave)
    : _modulation {wave.ChangeAmount >= 0.0f
                       ? 1.0 - (std::pow(static_cast<f64>(wave.ChangeAmount), 2.0) * 0.9)
                       : 1.0 + (std::pow(static_cast<f64>(wave.ChangeAmount), 2.0) * 10.0)}
    , _limit {static_cast<i32>((std::pow(1.0f - wave.ChangeSpeed, 2.0f) * 20000) + 32)}
{
}

auto arpeggio::operator()(f64 fperiod) -> f64
{
    ++_time;

    if (_limit != 0 && _time >= _limit) {
        _limit = 0;
        return fperiod * _modulation;
    }

    return fperiod;
}

////////////////////////////////////////////////////////////

square_duty::square_duty(sound_wave const& wave)
    : _squareDuty {0.5f - (wave.SquareDuty * 0.5f)}
    , _squareSlide {-wave.DutySweep * 0.00005f}
{
}

auto square_duty::operator()() -> f32
{
    _squareDuty += _squareSlide;
    _squareDuty = std::clamp(_squareDuty, 0.0f, 0.5f);
    return _squareDuty;
}

////////////////////////////////////////////////////////////

period::period(sound_wave const& wave)
    : _period {100.0 / (wave.StartFrequency * wave.StartFrequency + 0.001)}
    , _maxperiod {100.0 / (wave.MinFrequency * wave.MinFrequency + 0.001)}
    , _slide {1.0 - (std::pow(static_cast<f64>(wave.Slide), 3.0) * 0.01)}
    , _deltaSlide {-std::pow(static_cast<f64>(wave.DeltaSlide), 3.0) * 0.000001}
    , _arpeggio {wave}
{
}

auto period::operator()() -> f64
{
    FrequencyOutOfBounds = false;

    _period = _arpeggio(_period);

    _slide += _deltaSlide;
    _period *= _slide;

    if (_period > _maxperiod) {
        _period = _maxperiod;
        if (_maxperiod < 100000) {
            FrequencyOutOfBounds = true;
        }
    }

    return _period;
}

} // namespace audio
