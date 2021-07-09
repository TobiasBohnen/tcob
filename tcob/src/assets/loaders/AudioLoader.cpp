// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AudioLoader.hpp"

#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {
MusicLoader::MusicLoader(ResourceGroup& group)
    : ResourceLoader<Music> { group }
{
}

void MusicLoader::register_wrapper(lua::Script& script)
{
    // texture
    _funcNew = [this](const std::string& s) {
        auto music { get_or_create_resource(s) };
        auto def { std::make_unique<MusicDef>() };
        def->Res = music;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["music"] = _funcNew;

    auto& wrapper { script.create_wrapper<MusicDef>("MusicDef") };
    wrapper.function("source", [](MusicDef* def, const std::string& val) {
        def->info.source = val;
        return def;
    });
}

void MusicLoader::do_unload(ResourcePtr<Music> res, bool)
{
    _reloadInfo.erase(res.get()->name());
}

auto MusicLoader::do_reload(ResourcePtr<Music> res) -> bool
{
    if (!_reloadInfo.contains(res.get()->name())) {
        return false;
    }

    auto& info { _reloadInfo[res.get()->name()] };
    return res->open(group().mount_point() + info.source);
}

void MusicLoader::on_preparing()
{
    for (auto& def : _cache) {
        def->Res->open(group().mount_point() + def->info.source);

        _reloadInfo[def->Res.get()->name()] = def->info;

        set_resource_loaded(def->Res);
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

SoundLoader::SoundLoader(ResourceGroup& group)
    : ResourceLoader<Sound> { group }
{
}

void SoundLoader::register_wrapper(lua::Script& script)
{
    // texture
    _funcNew = [this](const std::string& s) {
        auto font { get_or_create_resource(s) };
        auto def { std::make_unique<SoundDef>() };
        def->Res = font;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["sound"] = _funcNew;

    auto& wrapper { script.create_wrapper<SoundDef>("SoundDef") };
    wrapper.function("source", [](SoundDef* def, const std::string& val) {
        def->info.source = val;
        return def;
    });
}

void SoundLoader::do_unload(ResourcePtr<Sound> res, bool)
{
    _reloadInfo.erase(res.get()->name());
}

auto SoundLoader::do_reload(ResourcePtr<Sound> res) -> bool
{
    if (!_reloadInfo.contains(res.get()->name())) {
        return false;
    }

    auto& info { _reloadInfo[res.get()->name()] };
    return res->load(group().mount_point() + info.source);
}

void SoundLoader::on_preparing()
{
    for (auto& def : _cache) {
        def->Res->load(group().mount_point() + def->info.source);

        _reloadInfo[def->Res.get()->name()] = def->info;

        set_resource_loaded(def->Res);
    }

    _cache.clear();
}

}