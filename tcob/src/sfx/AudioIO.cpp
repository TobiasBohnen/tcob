#include "AudioIO.hpp"

namespace tcob::detail {

auto read(void* userdata, void* buffer, isize bytesToRead) -> isize
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    return stream->read(reinterpret_cast<byte*>(buffer), bytesToRead);
}

auto seek_wav(void* userdata, i32 offset, drwav_seek_origin origin) -> drflac_bool32
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    auto dir { origin == drwav_seek_origin_current ? std::ios_base::cur : std::ios_base::beg };
    return stream->seek(offset, dir);
}

auto seek_flac(void* userdata, i32 offset, drflac_seek_origin origin) -> drflac_bool32
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    auto dir { origin == drflac_seek_origin_current ? std::ios_base::cur : std::ios_base::beg };
    return stream->seek(offset, dir);
}

auto seek_mp3(void* userdata, i32 offset, drmp3_seek_origin origin) -> drmp3_bool32
{
    InputFileStream* stream { reinterpret_cast<InputFileStream*>(userdata) };
    auto dir { origin == drmp3_seek_origin_current ? std::ios_base::cur : std::ios_base::beg };
    return stream->seek(offset, dir);
}

} // namespace detail
