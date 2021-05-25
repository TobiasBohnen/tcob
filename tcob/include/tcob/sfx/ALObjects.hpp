// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

namespace tcob::al {

class Buffer final {
public:
    Buffer();
    ~Buffer();

    void buffer_data(i32 channels, const void* data, i32 size, i32 freq) const;

    auto frequency() const -> i32;

    auto size() const -> i32;

    auto bits() const -> i32;

    auto channels() const -> i32;

    u32 ID { 0 };
};

////////////////////////////////////////////////////////////

class Source final {
public:
    Source();
    ~Source();

    void play();
    void stop();

    void buffer(u32 bufferID);

    auto pitch() const -> f32;
    void pitch(f32 value) const;

    auto gain() const -> f32;
    void gain(f32 value) const;

    auto position() const -> std::array<f32, 3>;
    void position(const std::array<f32, 3>& value) const;

    auto direction() const -> std::array<f32, 3>;
    void direction(const std::array<f32, 3>& value) const;

    auto rolloff_factor() const -> f32;
    void rolloff_factor(f32 value) const;

    auto source_relatvie() const -> bool;
    void source_relatvie(bool value) const;

    u32 ID { 0 };
};

}