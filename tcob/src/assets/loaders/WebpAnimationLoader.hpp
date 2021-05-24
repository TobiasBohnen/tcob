// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/drawables/WebpAnimation.hpp>

namespace tcob::detail {
class WebpAnimationLoader : public ResourceLoader<WebpAnimation> {
public:
    explicit WebpAnimationLoader(ResourceGroup& group);

    void register_wrapper(LuaScript& script) override;

protected:
    void do_unload(ResourcePtr<WebpAnimation> res, bool greedy) override;
    auto do_reload(ResourcePtr<WebpAnimation> res) -> bool override;

    void on_preparing() override;

private:
    struct ReloadInfo {
        std::string source;
        std::string material;
    };

    struct WebpAnimationDef {
        ResourcePtr<WebpAnimation> Res;
        ReloadInfo info;
    };

    std::function<WebpAnimationDef*(const std::string&)> _funcNew;
    std::vector<std::unique_ptr<WebpAnimationDef>> _cache;

    std::unordered_map<std::string, ReloadInfo> _reloadInfo;
};
}