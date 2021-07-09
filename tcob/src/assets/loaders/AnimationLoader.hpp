// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/Animation.hpp>

namespace tcob::detail {
class AnimationLoader : public ResourceLoader<FrameAnimation> {
public:
    explicit AnimationLoader(ResourceGroup& group);

    void register_wrapper(lua::Script& script) override;

protected:
    void on_preparing() override;

private:
    struct AnimationDef {
        ResourcePtr<FrameAnimation> Res;
    };

    std::function<AnimationDef*(const std::string&)> _funcNew;
    std::vector<std::unique_ptr<AnimationDef>> _cache;
};
}