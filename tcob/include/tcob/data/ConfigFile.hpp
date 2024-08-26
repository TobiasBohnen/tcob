// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Size.hpp"
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

struct video_config {
    bool   FullScreen {true};
    bool   UseDesktopResolution {true};
    size_i Resolution {};
    i32    FrameLimit {6000};
    bool   VSync {false};
#if defined(__EMSCRIPTEN__)
    string RenderSystem {"OPENGLES30"};
#else
    string RenderSystem {"OPENGL45"};
#endif

    void static Serialize(video_config const& v, auto&& s)
    {
        s[Cfg::Video::fullscreen]             = v.FullScreen;
        s[Cfg::Video::use_desktop_resolution] = v.UseDesktopResolution;
        s[Cfg::Video::resolution]             = v.Resolution;
        s[Cfg::Video::frame_limit]            = v.FrameLimit;
        s[Cfg::Video::vsync]                  = v.VSync;
        s[Cfg::Video::render_system]          = v.RenderSystem;
    }

    auto static Deserialize(video_config& v, auto&& s) -> bool
    {
        return s.try_get(v.FullScreen, Cfg::Video::fullscreen)
            && s.try_get(v.UseDesktopResolution, Cfg::Video::use_desktop_resolution)
            && s.try_get(v.Resolution, Cfg::Video::resolution)
            && s.try_get(v.FrameLimit, Cfg::Video::frame_limit)
            && s.try_get(v.VSync, Cfg::Video::vsync)
            && s.try_get(v.RenderSystem, Cfg::Video::render_system);
    }
};

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
