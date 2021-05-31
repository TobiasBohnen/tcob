// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <mutex>
#include <thread>

#include <tcob/core/io/FileStream.hpp>
#include <tcob/sfx/ALObjects.hpp>

namespace tcob {
namespace detail {
    class AudioDecoder {
    };
}

constexpr u32 MUSIC_BUFFER_SIZE { 4096 };
constexpr u32 MUSIC_BUFFER_COUNT { 4 };

class Music final {
public:
    Music();
    ~Music();

    auto open(const std::string& filename) -> bool;

    void play();
    void stop();

private:
    std::array<al::Buffer, MUSIC_BUFFER_COUNT> _buffers;
    std::unique_ptr<al::Source> _source;

    std::string _file {};
    std::unique_ptr<InputFileStream> _stream;
    std::thread _streamThread;
    std::mutex _mutex;
};
}