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
constexpr u32 MUSIC_BUFFER_SIZE { 4096 };
constexpr u32 MUSIC_BUFFER_COUNT { 4 };

namespace detail {

    class AudioDecoder {
    public:
        explicit AudioDecoder(const std::string& filename);
        virtual ~AudioDecoder() = default;

        auto buffer_data(u32 buffer) -> bool;

    protected:
        virtual auto info() const -> AudioInfo = 0;
        virtual auto read_data(i16* data, i32& frameCount) -> bool = 0;
        auto stream() const -> InputFileStream*;

    private:
        std::unique_ptr<InputFileStream> _stream;
    };
}

class Music final {
public:
    Music();
    ~Music();

    auto open(const std::string& filename) -> bool;

    void play();
    void stop();

private:
    void update_stream();
    void queue_buffers(const std::vector<u32>& buffers);

    std::array<al::Buffer, MUSIC_BUFFER_COUNT> _buffers;
    std::unique_ptr<al::Source> _source;

    std::unique_ptr<detail::AudioDecoder> _decoder;
    std::string _file {};

    std::thread _thread;
    std::recursive_mutex _mutex {};
};
}