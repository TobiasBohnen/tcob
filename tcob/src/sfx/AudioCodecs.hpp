// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#define DR_FLAC_NO_STDIO
#include <dr_libs/dr_flac.h>
#define DR_MP3_NO_STDIO
#include <dr_libs/dr_mp3.h>
#define DR_WAV_NO_STDIO
#include <dr_libs/dr_wav.h>
#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_PUSHDATA_API
#include <stb/stb_vorbis.c>

#include <tcob/sfx/Music.hpp>

namespace tcob::detail {
class AudioDecoder {
public:
    explicit AudioDecoder(const std::string& filename);
    virtual ~AudioDecoder() = default;

    auto buffer_data(al::Buffer* buffer) -> bool;

    virtual auto info() const -> AudioInfo = 0;

    virtual auto seek(f32 pos) -> bool = 0;

protected:
    auto stream() const -> InputFileStream*;
    virtual auto read_data(f32* data, isize size) -> i32 = 0;

private:
    std::unique_ptr<InputFileStream> _stream;
};

////////////////////////////////////////////////////////////

class WavDecoder final : public AudioDecoder {
public:
    WavDecoder(const std::string& filename);
    ~WavDecoder();

    auto info() const -> AudioInfo override;

    auto seek(f32 duration) -> bool override;

protected:
    auto read_data(f32* data, isize size) -> i32 override;

private:
    AudioInfo _info;
    drwav _wav;
};

////////////////////////////////////////////////////////////

class FlacDecoder final : public AudioDecoder {
public:
    FlacDecoder(const std::string& filename);
    ~FlacDecoder();

    auto info() const -> AudioInfo override;

    auto seek(f32 duration) -> bool override;

protected:
    auto read_data(f32* data, isize size) -> i32 override;

private:
    AudioInfo _info;
    drflac* _flac { nullptr };
};

////////////////////////////////////////////////////////////

class Mp3Decoder final : public AudioDecoder {
public:
    Mp3Decoder(const std::string& filename);
    ~Mp3Decoder();

    auto info() const -> AudioInfo override;

    auto seek(f32 duration) -> bool override;

protected:
    auto read_data(f32* data, isize size) -> i32 override;

private:
    AudioInfo _info;
    drmp3 _mp3;
};

////////////////////////////////////////////////////////////

class VorbisDecoder final : public AudioDecoder {
public:
    VorbisDecoder(const std::string& filename);
    ~VorbisDecoder();

    auto info() const -> AudioInfo override;

    auto seek(f32 duration) -> bool override;

protected:
    auto read_data(f32* data, isize size) -> i32 override;

private:
    AudioInfo _info;
    stb_vorbis* _vorbis { nullptr };
    i32 _headerBytes { 0 };
};
}