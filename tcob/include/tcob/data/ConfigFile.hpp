// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/data/ConfigTypes.hpp"

namespace tcob::Cfg::Video {
char const* const Name {"video"};
char const* const fullscreen {"fullscreen"};
char const* const use_desktop_resolution {"use_desktop_resolution"};
char const* const resolution {"resolution"};
char const* const frame_limit {"frame_limit"};
char const* const vsync {"vsync"};
char const* const render_system {"render_system"};
}

namespace tcob::data {
////////////////////////////////////////////////////////////

class TCOB_API config_file final : public data::config::object {
public:
    explicit config_file(string file);
    config_file(config_file const&)                    = delete;
    auto operator=(config_file const&) -> config_file& = delete;
    ~config_file() override;

    void save() const;

private:
    string _fileName;
};
}
