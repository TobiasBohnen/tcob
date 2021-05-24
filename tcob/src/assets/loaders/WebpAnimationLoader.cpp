// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "WebpAnimationLoader.hpp"

#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {

WebpAnimationLoader::WebpAnimationLoader(ResourceGroup& group)
    : ResourceLoader<WebpAnimation> { group }
{
}

void WebpAnimationLoader::register_wrapper(LuaScript& script)
{
    // animation
    _funcNew = [this](const std::string& s) {
        auto animation { get_or_create_resource(s) };
        auto def { std::make_unique<WebpAnimationDef>() };
        def->Res = animation;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };

    script.global_table()["webp_animation"] = _funcNew;

    auto& wrapper { script.create_wrapper<WebpAnimationDef>("WebpAnimationDef") };
    wrapper.function("source", [](WebpAnimationDef* def, const std::string& val) {
        def->info.source = val;
        return def;
    });
    wrapper.function("material", [](WebpAnimationDef* def, const std::string& val) {
        def->info.material = val;
        return def;
    });
}

void WebpAnimationLoader::do_unload(ResourcePtr<WebpAnimation> res, bool greedy)
{
    if (greedy)
        res->material().get()->unload(true);

    _reloadInfo.erase(res.get()->name());
}

auto WebpAnimationLoader::do_reload(ResourcePtr<WebpAnimation> res) -> bool
{
    if (!_reloadInfo.contains(res.get()->name())) {
        return false;
    }

    auto& info { _reloadInfo[res.get()->name()] };
    res->material(group().get<Material>(info.material));
    return res->load(group().mount_point() + info.source);
}

void WebpAnimationLoader::on_preparing()
{
    for (auto& def : _cache) {
        def->Res->material(group().get<Material>(def->info.material));
        def->Res->load(group().mount_point() + def->info.source);

        _reloadInfo[def->Res.get()->name()] = def->info;

        set_resource_loaded(def->Res);
    }
    _cache.clear();
}

}