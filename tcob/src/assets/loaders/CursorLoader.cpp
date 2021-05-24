// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CursorLoader.hpp"

#include <tcob/script/LuaScript.hpp>
#include <tcob/script/LuaTable.hpp>

namespace tcob::detail {
CursorLoader::CursorLoader(ResourceGroup& group)
    : ResourceLoader<Cursor> { group }
{
}

void CursorLoader::register_wrapper(LuaScript& script)
{
    // cursor
    _funcNew = [this](const std::string& s) {
        auto cursor { get_or_create_resource(s) };
        auto def { std::make_unique<CursorDef>() };
        def->Res = cursor;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };

    script.global_table()["cursor"] = _funcNew;

    auto& wrapper { script.create_wrapper<CursorDef>("CursorDef") };
    wrapper.function("material", [](CursorDef* def, const std::string& material) {
        def->material = material;
        return def;
    });
    wrapper.function("modes", [](CursorDef* def, const LuaTable& table) {
        for (auto& key : table.keys<std::string>()) {
            def->Res->define_mode(key, table[key]["texture"], table[key]["hotspot"]);
        }
        return def;
    });
}

void CursorLoader::on_preparing()
{
    for (auto& def : _cache) {
        def->Res->material(group().get<Material>(def->material));
        set_resource_loaded(def->Res);
    }

    _cache.clear();
}

void CursorLoader::do_unload(ResourcePtr<Cursor> res, bool greedy)
{
    if (greedy)
        res->material().get()->unload(true);
}
}