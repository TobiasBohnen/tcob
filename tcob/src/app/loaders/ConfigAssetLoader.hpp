// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <future>
#include <memory>
#include <unordered_map>
#include <vector>

#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/core/assets/AssetLoader.hpp"

#include "tcob/audio/Music.hpp"
#include "tcob/audio/Sound.hpp"
#include "tcob/audio/synth/SoundFont.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/FontFamily.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/animation/Animation.hpp"
#include "tcob/gfx/drawables/Cursor.hpp"

namespace tcob::detail {
////////////////////////////////////////////////////////////

class cfg_frame_animation_loader final : public assets::loader<gfx::frame_animation> {
public:
    cfg_frame_animation_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<gfx::frame_animation> assetPtr;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;
    data::object&                           _object;
};

////////////////////////////////////////////////////////////

class cfg_music_loader final : public assets::loader<audio::music> {
public:
    cfg_music_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<audio::music> assetPtr;
        string                          source;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;
    data::object&                           _object;
};

////////////////////////////////////////////////////////////

class cfg_sound_loader final : public assets::loader<audio::sound> {
public:
    cfg_sound_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<audio::sound> assetPtr;
        std::future<bool>               future;
        string                          source;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;
    data::object&                           _object;
};

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)

class cfg_sound_font_loader final : public assets::loader<audio::sound_font> {
public:
    cfg_sound_font_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<audio::sound_font> assetPtr;
        std::future<bool>                    future;
        string                               source;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;
    data::object&                           _object;
};

#endif

////////////////////////////////////////////////////////////

class cfg_cursor_loader final : public assets::loader<gfx::cursor> {
public:
    cfg_cursor_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<gfx::cursor> assetPtr;
        string                         material;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;
    data::object&                           _object;
};

////////////////////////////////////////////////////////////

class cfg_font_loader final : public assets::loader<gfx::font> {
public:
    cfg_font_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<gfx::font> assetPtr;
        string                       source;
        u32                          size {0};
    };

    std::vector<std::unique_ptr<asset_def>> _cache;

    data::object& _object;
};

////////////////////////////////////////////////////////////

class cfg_font_family_loader final : public assets::loader<gfx::font_family> {
public:
    cfg_font_family_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<gfx::font_family> assetPtr;
        string                              source;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;
    data::object&                           _object;
};

////////////////////////////////////////////////////////////

class cfg_material_loader final : public assets::loader<gfx::material> {
public:
    cfg_material_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<gfx::material> assetPtr;
        string                           shader;
        string                           texture;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;
    data::object&                           _object;
};

////////////////////////////////////////////////////////////

class cfg_shader_loader final : public assets::loader<gfx::shader> {
public:
    cfg_shader_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    struct asset_def {
        assets::asset_ptr<gfx::shader> assetPtr;
        string                         vertex;
        string                         fragment;
    };

    std::vector<std::unique_ptr<asset_def>> _cache;

    data::object& _object;
};

////////////////////////////////////////////////////////////

class cfg_texture_loader final : public assets::loader<gfx::texture> {
public:
    cfg_texture_loader(assets::group& group, data::object& object);

    void declare() override;
    void prepare() override;

private:
    void check_async_load(def_task const& ctx);

    // texture
    struct image_ftr {
        u32               Depth {};
        path              Path {};
        gfx::image        Image {};
        std::future<bool> Future {};
    };

    struct tex_asset_def {
        assets::asset_ptr<gfx::texture>                 assetPtr;
        gfx::texture::filtering                         filtering {gfx::texture::filtering::NearestNeighbor};
        gfx::texture::wrapping                          wrapping {gfx::texture::wrapping::Repeat};
        size_i                                          size {size_i::Zero};
        std::unordered_map<string, gfx::texture_region> abs_regions;

        std::vector<image_ftr> images;
    };

    std::vector<std::unique_ptr<tex_asset_def>> _cacheTex;

    // animated texture
    struct ani_asset_def {
        assets::asset_ptr<gfx::texture> assetPtr;
        gfx::texture::filtering         filtering {gfx::texture::filtering::NearestNeighbor};
        gfx::texture::wrapping          wrapping {gfx::texture::wrapping::Repeat};
        path                            textureFile;
    };

    std::vector<std::unique_ptr<ani_asset_def>> _cacheAni;

    //
    data::object& _object;
};

////////////////////////////////////////////////////////////

class cfg_asset_loader_manager final : public assets::loader_manager {
public:
    explicit cfg_asset_loader_manager(assets::group& group);

    void load_script(path const& file) override;

private:
    data::object _object;
};

}
