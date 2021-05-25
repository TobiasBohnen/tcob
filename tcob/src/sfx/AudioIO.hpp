// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include <tcob/core/io/FileStream.hpp>

#define DR_FLAC_NO_STDIO
#include <dr_libs/dr_flac.h>
#define DR_MP3_NO_STDIO
#include <dr_libs/dr_mp3.h>
#define DR_WAV_NO_STDIO
#include <dr_libs/dr_wav.h>

namespace tcob::detail {
auto read(void* userdata, void* buffer, isize bytesToRead) -> isize;

auto seek_wav(void* userdata, i32 offset, drwav_seek_origin origin) -> u32;

auto seek_flac(void* userdata, i32 offset, drflac_seek_origin origin) -> u32;

auto seek_mp3(void* userdata, i32 offset, drmp3_seek_origin origin) -> u32;
}