// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/drawables/Cursor.hpp>

namespace tcob::detail {
class CursorLoader : public ResourceLoader<Cursor> {
public:
    explicit CursorLoader(ResourceGroup& group);

    void register_wrapper(LuaScript& script) override;

protected:
    void on_preparing() override;
    void do_unload(ResourcePtr<Cursor> res, bool greedy) override;

private:
    struct CursorDef {
        ResourcePtr<Cursor> Res;
        std::string material;
    };

    std::function<CursorDef*(const std::string&)> _funcNew;
    std::vector<std::unique_ptr<CursorDef>> _cache;
};
}