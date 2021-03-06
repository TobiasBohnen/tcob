// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <atomic>
#include <mutex>
#include <span>
#include <thread>

#include <tcob/core/io/FileStream.hpp>
#include <tcob/sfx/ALObjects.hpp>
#include <tcob/sfx/AudioSource.hpp>

namespace tcob {
constexpr u32 MUSIC_BUFFER_SIZE { 4096 };
constexpr u32 MUSIC_BUFFER_COUNT { 4 };

namespace detail {
    class AudioDecoder;
}

class Music final : public AudioSource {
public:
    Music();
    ~Music();

    Music(const Music& other);
    auto operator=(const Music& other) -> Music&;

    auto open(const std::string& filename) -> bool;

    void start(bool looped = false) override;
    void stop() override;

    auto duration() const -> MilliSeconds override;
    auto playback_position() const -> MilliSeconds override;

private:
    void update_stream();
    void stop_stream();
    void queue_buffers(std::span<u32> buffers);
    void fill_buffers();

    std::array<std::shared_ptr<al::Buffer>, MUSIC_BUFFER_COUNT> _buffers;

    std::unique_ptr<detail::AudioDecoder> _decoder;
    std::string _file {};

    std::thread _thread;
    std::atomic_bool _requestStop { false };

    i32 _samplesPlayed { 0 };
};
}