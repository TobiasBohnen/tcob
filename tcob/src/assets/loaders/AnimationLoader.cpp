// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AnimationLoader.hpp"

#include <vector>

#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {

static const std::unordered_map<std::string, AnimationPlaybackMode> playbackmode {
    { "Normal", AnimationPlaybackMode::Normal },
    { "Reversed", AnimationPlaybackMode::Reversed },
    { "Looped", AnimationPlaybackMode::Looped },
    { "ReversedLooped", AnimationPlaybackMode::ReversedLooped },
    { "Alternated", AnimationPlaybackMode::Alternated },
    { "AlternatedLooped", AnimationPlaybackMode::AlternatedLooped },
};

AnimationLoader::AnimationLoader(ResourceGroup& group)
    : ResourceLoader<FrameAnimation> { group }
{
}

void AnimationLoader::register_wrapper(lua::Script& script)
{
    // animation
    _funcNew = [this](const std::string& s) {
        auto animation { get_or_create_resource(s) };
        auto def { std::make_unique<AnimationDef>() };
        def->Res = animation;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };

    script.global_table()["animation"] = _funcNew;

    auto& wrapper { script.create_wrapper<AnimationDef>("AnimationDef") };
    wrapper.function("frames", [](AnimationDef* def, const std::vector<std::string>& val) {
        def->Res->Frames = val;
        return def;
    });
    wrapper.function("duration", [](AnimationDef* def, f64 val) {
        def->Res->Duration = MilliSeconds { val };
        return def;
    });
    wrapper.function("playback_mode", [](AnimationDef* def, const std::string& val) {
        def->Res->Mode = playbackmode.at(val);
        return def;
    });
}

void AnimationLoader::on_preparing()
{
    for (auto& def : _cache)
        set_resource_loaded(def->Res);
    _cache.clear();
}

}