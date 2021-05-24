// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FontLoader.hpp"

#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {
FontLoader::FontLoader(ResourceGroup& group)
    : ResourceLoader<Font> { group }
{
}

void FontLoader::register_wrapper(LuaScript& script)
{
    // texture
    _funcNew = [this](const std::string& s) {
        auto font { get_or_create_resource(s) };
        auto def { std::make_unique<FontDef>() };
        def->Res = font;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["font"] = _funcNew;

    auto& wrapper { script.create_wrapper<FontDef>("FontDef") };
    wrapper.function("source", [](FontDef* def, const std::string& val) {
        def->info.source = val;
        return def;
    });
    wrapper.function("size", [](FontDef* def, u32 val) {
        def->info.size = val;
        return def;
    });
    wrapper.function("kerning", [](FontDef* def, bool val) {
        def->kerning = val;
        return def;
    });
    wrapper.function("is_default", [](FontDef* def) {
        def->isDefault = true;
        return def;
    });
}

void FontLoader::do_unload(ResourcePtr<Font> res, bool)
{
    _reloadInfo.erase(res.get()->name());
}

auto FontLoader::do_reload(ResourcePtr<Font> res) -> bool
{
    if (!_reloadInfo.contains(res.get()->name())) {
        return false;
    }

    auto& info { _reloadInfo[res.get()->name()] };
    return res->load(group().mount_point() + info.source, info.size);
}

void FontLoader::on_preparing()
{
    for (auto& def : _cache) {
        def->Res->load(group().mount_point() + def->info.source, def->info.size);
        def->Res->kerning(def->kerning);
        if (def->isDefault) {
            Font::Default = def->Res;
        }
        _reloadInfo[def->Res.get()->name()] = def->info;

        set_resource_loaded(def->Res);
    }

    _cache.clear();
}
}