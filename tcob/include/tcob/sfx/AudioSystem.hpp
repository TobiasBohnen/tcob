// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

namespace tcob {
class AudioSystem final {
public:
    AudioSystem();
    ~AudioSystem();

private:
    ALCdevice* _device;
    ALCcontext* _context;
};
}