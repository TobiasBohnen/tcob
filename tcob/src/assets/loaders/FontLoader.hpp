// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/Font.hpp>

namespace tcob::detail {
class FontLoader : public ResourceLoader<Font> {
public:
    explicit FontLoader(ResourceGroup& group);

    void register_wrapper(LuaScript& script) override;

protected:
    void do_unload(ResourcePtr<Font> res, bool greedy) override;
    auto do_reload(ResourcePtr<Font> res) -> bool override;

    void on_preparing() override;

private:
    struct ReloadInfo {
        std::string source;
        u32 size;
    };

    struct FontDef {
        ResourcePtr<Font> Res;
        ReloadInfo info;
        std::string material;
        bool kerning { false };
        bool isDefault { false };
        std::optional<f32> linegap;
    };

    std::function<FontDef*(const std::string&)> _funcNewSDF;
    std::function<FontDef*(const std::string&)> _funcNewTTF;
    std::vector<std::unique_ptr<FontDef>> _cache;

    std::unordered_map<std::string, ReloadInfo> _reloadInfo;
};
}