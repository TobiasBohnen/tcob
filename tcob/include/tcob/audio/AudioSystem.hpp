// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"

using ALCdevice  = struct ALCdevice_struct;
using ALCcontext = struct ALCcontext_struct;

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API system final : public non_copyable {
public:
    system();
    ~system();

    static inline char const* service_name {"audio_system"};

private:
    ALCdevice*  _device;
    ALCcontext* _context;
};

}
