// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/sfx/Music.hpp>
#include <tcob/sfx/Sound.hpp>

namespace tcob::detail {

class MusicLoader : public ResourceLoader<Music> {
public:
    explicit MusicLoader(ResourceGroup& group);

    void register_wrapper(lua::Script& script) override;

protected:
    void do_unload(ResourcePtr<Music> res, bool greedy) override;
    auto do_reload(ResourcePtr<Music> res) -> bool override;

    void on_preparing() override;

private:
    struct ReloadInfo {
        std::string source;
    };

    struct MusicDef {
        ResourcePtr<Music> Res;
        ReloadInfo info;
    };

    std::function<MusicDef*(const std::string&)> _funcNew;
    std::vector<std::unique_ptr<MusicDef>> _cache;

    std::unordered_map<std::string, ReloadInfo> _reloadInfo;
};

////////////////////////////////////////////////////////////

class SoundLoader : public ResourceLoader<Sound> {
public:
    explicit SoundLoader(ResourceGroup& group);

    void register_wrapper(lua::Script& script) override;

protected:
    void do_unload(ResourcePtr<Sound> res, bool greedy) override;
    auto do_reload(ResourcePtr<Sound> res) -> bool override;

    void on_preparing() override;

private:
    struct ReloadInfo {
        std::string source;
    };

    struct SoundDef {
        ResourcePtr<Sound> Res;
        ReloadInfo info;
    };

    std::function<SoundDef*(const std::string&)> _funcNew;
    std::vector<std::unique_ptr<SoundDef>> _cache;

    std::unordered_map<std::string, ReloadInfo> _reloadInfo;
};
}