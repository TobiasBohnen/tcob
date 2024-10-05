// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigAssetLoader.hpp"

#include <utility>

#include "tcob/app/SerializeHelper.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/RenderSystem.hpp"

using namespace tcob::gfx;
using namespace tcob::audio;
using namespace tcob::data::config;
using namespace tcob::assets;

using namespace std::chrono_literals;

namespace io = tcob::io;

namespace tcob::API {
namespace Animation {
    static char const* Name {"animation"};
    static char const* frames {"frames"};
}

namespace Music {
    static char const* Name {"music"};
    static char const* source {"source"};
}

namespace Sound {
    static char const* Name {"sound"};
    static char const* source {"source"};
}

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)
namespace SoundFont {
    static char const* Name {"sound_font"};
    static char const* source {"source"};
}
#endif

namespace Cursor {
    static char const* Name {"cursor"};
    static char const* material {"material"};
    static char const* modes {"modes"};
}

namespace TrueTypeFont {
    static char const* Name {"font"};
    static char const* source {"source"};
    static char const* size {"size"};
}

namespace FontFamily {
    static char const* Name {"font_family"};
    static char const* source {"source"};
}

namespace Material {
    static char const* Name {"material"};
    static char const* texture {"texture"};
    static char const* shader {"shader"};
    static char const* blend_func {"blend_func"};
    static char const* separate_blend_func {"separate_blend_func"};
    static char const* blend_equation {"blend_equation"};
    static char const* point_size {"point_size"};
    static char const* color {"color"};
}

namespace Shader {
    static char const* Name {"shader"};
    static char const* vertex {"vertex"};
    static char const* fragment {"fragment"};
}

namespace Texture {
    static char const* Name {"texture"};
    static char const* source {"source"};
    static char const* xy_regions {"xy_regions"};
    static char const* uv_regions {"uv_regions"};
    static char const* size {"size"};
    static char const* wrapping {"wrapping"};
    static char const* filtering {"filtering"};
}

namespace AnimatedTexture {
    static char const* Name {"animated_texture"};
    static char const* source {"source"};
    static char const* wrapping {"wrapping"};
    static char const* filtering {"filtering"};
}
}

namespace tcob::detail {

template <typename T, typename TDef, typename Loader, typename Bucket, typename Cache>
auto default_new(string const& name, Loader* loader, Bucket* bucket, Cache& cache) -> TDef*
{
    auto def {std::make_unique<TDef>()};
    def->assetPtr = bucket->template create_or_get<T>(name, loader);

    auto const retValue {def.get()};
    cache.push_back(std::move(def));
    return retValue;
}

auto default_check_async_load(auto&& cache, auto&& stateSetter) -> task_status
{
    using namespace std::chrono_literals;

    if (cache.empty()) {
        return task_status::Finished;
    }

    bool loadingDone {true};

    for (auto it {cache.begin()}; it != cache.end(); ++it) {
        auto& def {*it};
        auto& ftr {def->future};
        if (ftr.valid()) {
            if (ftr.wait_for(0s) == std::future_status::ready) {
                if (ftr.get() == load_status::Ok) {
                    stateSetter(def->assetPtr, status::Loaded);
                } else {
                    stateSetter(def->assetPtr, status::Error);
                }
            } else {
                loadingDone = false;
            }
        }
    }

    if (loadingDone) {
        cache.clear();
    }

    return cache.empty() ? task_status::Finished : task_status::Running;
}

cfg_asset_loader_manager::cfg_asset_loader_manager(group& group)
{
    group.add_bucket<gfx::shader>();
    add_loader(std::make_unique<cfg_shader_loader>(group, _object));

    group.add_bucket<gfx::texture>();
    add_loader(std::make_unique<cfg_texture_loader>(group, _object));

    group.add_bucket<gfx::material>();
    add_loader(std::make_unique<cfg_material_loader>(group, _object));

    group.add_bucket<gfx::cursor>();
    add_loader(std::make_unique<cfg_cursor_loader>(group, _object));

    group.add_bucket<gfx::font>();
    add_loader(std::make_unique<cfg_font_loader>(group, _object));

    group.add_bucket<gfx::font_family>();
    add_loader(std::make_unique<cfg_font_family_loader>(group, _object));

    group.add_bucket<gfx::frame_animation>();
    add_loader(std::make_unique<cfg_frame_animation_loader>(group, _object));

    group.add_bucket<audio::music>();
    add_loader(std::make_unique<cfg_music_loader>(group, _object));

    group.add_bucket<audio::sound>();
    add_loader(std::make_unique<cfg_sound_loader>(group, _object));

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)
    group.add_bucket<audio::sound_font>();
    add_loader(std::make_unique<cfg_sound_font_loader>(group, _object));
#endif
}

void cfg_asset_loader_manager::load(path const& file)
{
    object load;
    if (load.load(file) == load_status::Ok) {
        _object.merge(load, true);
    } else {
        logger::Error("AssetLoader '{}': loading failed.", file);
    }
}

////////////////////////////////////////////////////////////

cfg_frame_animation_loader::cfg_frame_animation_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_frame_animation_loader::unload_asset(asset<gfx::frame_animation>& asset, bool)
{
    _object[API::Animation::Name][asset.get_name()] = nullptr;
}

void cfg_frame_animation_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Animation::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        if (object assetSection; v.try_get(assetSection)) {
            auto* asset {default_new<frame_animation, asset_def>(k, this, get_bucket(), _cache)};
            if (std::vector<frame> frames; assetSection.try_get(frames, API::Animation::frames)) {
                asset->assetPtr->Frames = frames;
            }
        }
    }
}

void cfg_frame_animation_loader::prepare()
{
    for (auto& def : _cache) {
        set_asset_status(def->assetPtr, status::Loaded);
    }
    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_music_loader::cfg_music_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_music_loader::unload_asset(asset<music>& asset, bool)
{
    _object[API::Music::Name][asset.get_name()] = nullptr;
}

auto cfg_music_loader::reload_asset(asset<music>& asset) -> bool
{
    if (!_object[API::Music::Name].as<object>().has(asset.get_name())) {
        return false;
    }

    string const& source {_object[API::Music::Name][asset.get_name()][API::Music::source].as<string>()};
    return asset->open(get_group().get_mount_point() + source) == load_status::Ok;
}

void cfg_music_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Music::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        auto* asset {default_new<music, asset_def>(k, this, get_bucket(), _cache)};
        if (object assetSection; v.try_get(assetSection)) {
            if (string source; assetSection.try_get(source, API::Music::source)) {
                asset->source = source;
            }
        } else if (path assetString; v.try_get(assetString)) {
            asset->source = assetString;
        }
    }
}

void cfg_music_loader::prepare()
{
    for (auto& def : _cache) {
        if (def->assetPtr->open(get_group().get_mount_point() + def->source) == load_status::Ok) {
            set_asset_status(def->assetPtr, status::Loaded);
        } else {
            set_asset_status(def->assetPtr, status::Error);
        }
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_sound_loader::cfg_sound_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_sound_loader::unload_asset(asset<sound>& asset, bool)
{
    _object[API::Sound::Name][asset.get_name()] = nullptr;
}

auto cfg_sound_loader::reload_asset(asset<sound>& asset) -> bool
{
    if (!_object[API::Sound::Name].as<object>().has(asset.get_name())) {
        return false;
    }

    string const& source {_object[API::Sound::Name][asset.get_name()][API::Sound::source].as<string>()};
    return asset->load(get_group().get_mount_point() + source) == load_status::Ok;
}

void cfg_sound_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Sound::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        auto* asset {default_new<sound, asset_def>(k, this, get_bucket(), _cache)};

        if (object assetSection; v.try_get(assetSection)) {
            if (string source; assetSection.try_get(source, API::Sound::source)) {
                asset->source = source;
            }
        } else if (path assetString; v.try_get(assetString)) {
            asset->source = assetString;
        }
    }
}

void cfg_sound_loader::prepare()
{
    for (auto& def : _cache) {
        def->future = def->assetPtr->load_async(get_group().get_mount_point() + def->source);
        set_asset_status(def->assetPtr, status::Loading);
    }

    locate_service<task_manager>().run_deferred([&]() {
        return default_check_async_load(_cache, [&](auto&& asset, auto&& state) { set_asset_status(asset, state); });
    });
}

////////////////////////////////////////////////////////////

#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)

cfg_sound_font_loader::cfg_sound_font_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_sound_font_loader::unload_asset(asset<sound_font>& asset, bool)
{
    _object[API::SoundFont::Name][asset.get_name()] = nullptr;
}

auto cfg_sound_font_loader::reload_asset(asset<sound_font>& asset) -> bool
{
    if (!_object[API::SoundFont::Name].as<object>().has(asset.get_name())) {
        return false;
    }

    string const& source {_object[API::SoundFont::Name][asset.get_name()][API::SoundFont::source].as<string>()};
    return asset->load(get_group().get_mount_point() + source) == load_status::Ok;
}

void cfg_sound_font_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::SoundFont::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        auto* asset {default_new<sound_font, asset_def>(k, this, get_bucket(), _cache)};

        if (object assetSection; v.try_get(assetSection)) {
            if (string source; assetSection.try_get(source, API::SoundFont::source)) {
                asset->source = source;
            }
        } else if (path assetString; v.try_get(assetString)) {
            asset->source = assetString;
        }
    }
}

void cfg_sound_font_loader::prepare()
{
    for (auto& def : _cache) {
        def->future = def->assetPtr->load_async(get_group().get_mount_point() + def->source);
        set_asset_status(def->assetPtr, status::Loading);
    }

    locate_service<task_manager>().run_deferred([&]() {
        return default_check_async_load(_cache, [&](auto&& asset, auto&& state) { set_asset_status(asset, state); });
    });
}

#endif

////////////////////////////////////////////////////////////

cfg_cursor_loader::cfg_cursor_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_cursor_loader::unload_asset(asset<cursor>& asset, bool greedy)
{
    _object[API::Cursor::Name][asset.get_name()] = nullptr;
    if (greedy) {
        asset->Material().get()->unload(true);
    }
}

void cfg_cursor_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Cursor::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        if (object assetSection; v.try_get(assetSection)) {
            auto* asset {default_new<cursor, asset_def>(k, this, get_bucket(), _cache)};
            if (string material; assetSection.try_get(material, API::Cursor::material)) {
                asset->material = material;
            }
            if (object modesSection; assetSection.try_get(modesSection, API::Cursor::modes)) {
                for (auto const& [mk, mv] : modesSection) {
                    if (object modeSection; mv.try_get(modeSection)) {
                        asset->assetPtr->add_mode(mk, modeSection["hotspot"].as<point_i>());
                    }
                }
            }
        }
    }
}

void cfg_cursor_loader::prepare()
{
    for (auto& def : _cache) {
        def->assetPtr->Material = get_group().get<material>(def->material);
        set_asset_status(def->assetPtr, status::Loaded);
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_font_loader::cfg_font_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_font_loader::unload_asset(asset<font>& asset, bool)
{
    auto const& name {asset.get_name()};
    if (_object[API::TrueTypeFont::Name].as<object>().has(name)) {
        _object[API::TrueTypeFont::Name][name] = nullptr;
    }
}

auto cfg_font_loader::reload_asset(asset<font>& asset) -> bool
{
    auto const& name {asset.get_name()};
    auto const  mp {get_group().get_mount_point()};
    if (_object[API::TrueTypeFont::Name].as<object>().has(name)) {
        auto  info {_object[API::TrueTypeFont::Name][name]};
        auto* ttf {dynamic_cast<font*>(asset.get())};
        if (ttf->load(mp + info[API::TrueTypeFont::source].as<path>(), info[API::TrueTypeFont::size].as<u32>()) != load_status::Ok) {
            logger::Error("font asset: Error reloading file: {}", info[API::TrueTypeFont::source].as<path>());
            return false;
        }

        return true;
    }

    return false;
}

void cfg_font_loader::declare()
{
    if (object fontSection; _object.try_get(fontSection, API::TrueTypeFont::Name)) {
        for (auto const& [k, v] : fontSection) {
            if (object assetSection; v.try_get(assetSection)) {
                auto* asset {default_new<font, asset_def>(k, this, get_bucket(), _cache)};
                if (string source; assetSection.try_get(source, API::TrueTypeFont::source)) {
                    asset->source = source;
                }
                if (u32 size {0}; assetSection.try_get(size, API::TrueTypeFont::size)) {
                    asset->size = size;
                }
            }
        }
    }
}

void cfg_font_loader::prepare()
{
    for (auto& def : _cache) {
        auto* ttf {dynamic_cast<font*>(def->assetPtr.get_obj())};
        if (ttf) {
            if (ttf->load(get_group().get_mount_point() + def->source, def->size) == load_status::Ok) {
                set_asset_status(def->assetPtr, status::Loaded);
            } else {
                set_asset_status(def->assetPtr, status::Error);
            }
        }
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_font_family_loader::cfg_font_family_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_font_family_loader::unload_asset(asset<gfx::font_family>& asset, bool)
{
    _object[API::FontFamily::Name][asset->get_name()] = nullptr;
}

void cfg_font_family_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::FontFamily::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        auto asset {std::make_unique<asset_def>()};
        asset->assetPtr = get_bucket()->template create_or_get<font_family>(k, this, k);

        if (object assetSection; v.try_get(assetSection)) {
            if (string fontSource; assetSection.try_get(fontSource, API::FontFamily::source)) {
                asset->source = fontSource;
            }
        } else if (path assetString; v.try_get(assetString)) {
            asset->source = assetString;
        }

        _cache.push_back(std::move(asset));
    }
}

void cfg_font_family_loader::prepare()
{
    auto& grp {get_group()};

    for (auto const& def : _cache) {
        font_family::FindSources(*def->assetPtr, grp.get_mount_point() + def->source);
        set_asset_status(def->assetPtr, status::Loaded);
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_material_loader::cfg_material_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_material_loader::unload_asset(asset<material>& asset, bool greedy)
{
    _object[API::Material::Name][asset.get_name()] = nullptr;

    if (greedy) {
        asset->Shader.get()->unload(true);
        asset->Texture.get()->unload(true);
    }
}

void cfg_material_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Material::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        if (object assetSection; v.try_get(assetSection)) {
            auto* asset {default_new<material, asset_def>(k, this, get_bucket(), _cache)};
            if (string texture; assetSection.try_get(texture, API::Material::texture)) {
                asset->texture = texture;
            }
            if (string shader; assetSection.try_get(shader, API::Material::shader)) {
                asset->shader = shader;
            }
            if (object blendFunc; assetSection.try_get(blendFunc, API::Material::blend_func)) {
                blend_func s {blendFunc["source"].as<blend_func>()};
                asset->assetPtr->BlendFuncs.SourceAlphaBlendFunc = s;
                asset->assetPtr->BlendFuncs.SourceColorBlendFunc = s;

                blend_func d {blendFunc["destination"].as<blend_func>()};
                asset->assetPtr->BlendFuncs.DestinationAlphaBlendFunc = d;
                asset->assetPtr->BlendFuncs.DestinationColorBlendFunc = d;
            } else if (object separateBlendFunc; assetSection.try_get(separateBlendFunc, API::Material::separate_blend_func)) {
                blend_func sa {separateBlendFunc["source_alpha"].as<blend_func>()};
                asset->assetPtr->BlendFuncs.SourceAlphaBlendFunc = sa;
                blend_func sc {separateBlendFunc["source_color"].as<blend_func>()};
                asset->assetPtr->BlendFuncs.SourceColorBlendFunc = sc;

                blend_func da {separateBlendFunc["destination_alpha"].as<blend_func>()};
                asset->assetPtr->BlendFuncs.DestinationAlphaBlendFunc = da;
                blend_func ds {separateBlendFunc["destination_color"].as<blend_func>()};
                asset->assetPtr->BlendFuncs.DestinationColorBlendFunc = ds;
            }
            if (blend_equation blendEquation {}; assetSection.try_get(blendEquation, API::Material::blend_equation)) {
                asset->assetPtr->BlendEquation = blendEquation;
            }
            if (color c {colors::White}; assetSection.try_get(c, API::Material::color)) {
                asset->assetPtr->Color = c;
            }
            if (f32 pointSize {0}; assetSection.try_get(pointSize, API::Material::point_size)) {
                asset->assetPtr->PointSize = pointSize;
            }
        }
    }
}

void cfg_material_loader::prepare()
{
    auto& grp {get_group()};

    for (auto const& def : _cache) {
        auto const& name {def->assetPtr.get()->get_name()};

        if (!def->shader.empty()) {
            if (!grp.has<shader>(def->shader)) {
                logger::Error("material asset '{}': Shader '{}' not found.", name, def->shader);
                set_asset_status(def->assetPtr, status::Error);
            } else {
                def->assetPtr->Shader = grp.get<shader>(def->shader);
            }
        }

        if (!def->texture.empty()) {
            if (!grp.has<texture>(def->texture)) {
                logger::Error("material asset '{}': Texture '{}' not found.", name, def->texture);
                set_asset_status(def->assetPtr, status::Error);
            } else {
                def->assetPtr->Texture = grp.get<texture>(def->texture);
            }
        }

        set_asset_status(def->assetPtr, status::Loaded);
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_shader_loader::cfg_shader_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_shader_loader::unload_asset(asset<shader>& asset, bool)
{
    _object[API::Shader::Name][asset.get_name()] = nullptr;
}

auto cfg_shader_loader::reload_asset(asset<shader>& asset) -> bool
{
    auto const& name {asset.get_name()};
    if (!_object[API::Shader::Name].as<object>().has(name)) {
        return false;
    }

    auto const vertSource {io::read_as_string(get_group().get_mount_point() + _object[API::Shader::Name][name][API::Shader::vertex].as<string>())};
    auto const fragSource {io::read_as_string(get_group().get_mount_point() + _object[API::Shader::Name][name][API::Shader::fragment].as<string>())};
    return asset->create(vertSource, fragSource);
}

void cfg_shader_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Shader::Name)) {
        return;
    }

    for (auto const& [k, v] : obj) {
        if (object assetSection; v.try_get(assetSection)) {
            // try getting render system specific shader
            assetSection.try_get(assetSection, locate_service<render_system>().get_name());

            auto* asset {default_new<shader, asset_def>(k, this, get_bucket(), _cache)};
            if (string vertex; assetSection.try_get(vertex, API::Shader::vertex)) {
                asset->vertex = vertex;
            }
            if (string fragment; assetSection.try_get(fragment, API::Shader::fragment)) {
                asset->fragment = fragment;
            }
        }
    }
}

void cfg_shader_loader::prepare()
{
    if (!_cache.empty()) {
        for (auto& def : _cache) {
            auto const vertSource {io::read_as_string(get_group().get_mount_point() + def->vertex)};
            if (vertSource.empty()) {
                logger::Error("shader asset '{}': Vertex shader '{}' not found.", def->assetPtr.get()->get_name(), def->vertex);
                set_asset_status(def->assetPtr, status::Error);
                continue;
            }
            auto const fragSource {io::read_as_string(get_group().get_mount_point() + def->fragment)};
            if (fragSource.empty()) {
                logger::Error("shader asset '{}': Fragment shader '{}' not found.", def->assetPtr.get()->get_name(), def->fragment);
                set_asset_status(def->assetPtr, status::Error);
                continue;
            }

            def->assetPtr->create(vertSource, fragSource);
            set_asset_status(def->assetPtr, status::Loaded);
        }

        _cache.clear();
    }
}

////////////////////////////////////////////////////////////

cfg_texture_loader::cfg_texture_loader(assets::group& group, data::config::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_texture_loader::unload_asset(asset<texture>& asset, bool)
{
    auto const& name {asset.get_name()};
    if (_object[API::Texture::Name].as<object>().has(name)) {
        _object[API::Texture::Name][name] = nullptr;
    }
    if (_object[API::AnimatedTexture::Name].as<object>().has(name)) {
        _object[API::AnimatedTexture::Name][name] = nullptr;
    }
}

auto cfg_texture_loader::reload_asset(asset<texture>& asset) -> bool
{
    auto const& name {asset.get_name()};
    if (_reloadInfoTex.contains(name)) {
        auto& info {_reloadInfoTex[name]};
        for (auto& img : info) {
            auto const  image = *image::Load(img.Path);
            auto const& imgInfo {image.get_info()};
            if (imgInfo.bytes_per_pixel() == 4) {
                asset->update_data(image.get_data(), img.Depth, 0, 4);
            } else {
                asset->update_data(image.get_data(), img.Depth, 0, 1);
            }
        }
        return true;
    }

    if (_object[API::AnimatedTexture::Name].as<object>().has(name)) {
        auto  info {_object[API::AnimatedTexture::Name][name]};
        auto* ani {dynamic_cast<animated_texture*>(asset.get())};
        return ani->load(get_group().get_mount_point() + info[API::AnimatedTexture::source].as<string>()) == load_status::Ok;
    }

    return false;
}

void cfg_texture_loader::declare()
{
    path const mp {get_group().get_mount_point()};

    if (object textureSection; _object.try_get(textureSection, API::Texture::Name)) {
        for (auto const& [k, v] : textureSection) {
            auto* asset {default_new<texture, tex_asset_def>(k, this, get_bucket(), _cacheTex)};
            asset->assetPtr->add_region("default", {{0.0f, 0.0f, 1.0f, 1.0f}, 0});

            // texture objects
            if (object assetSection; v.try_get(assetSection)) {
                std::vector<path> files;

                if (std::vector<path> items; assetSection.try_get(items, API::Texture::source)) {
                    for (auto const& item : items) {
                        path const f {mp + item};
                        if (io::exists(f)) { // file or folder
                            if (io::is_file(f)) {
                                files.push_back(f);
                            } else if (io::is_folder(f)) {
                                auto const moreFiles {io::enumerate(f, {"*.*"}, false)};
                                files.insert(files.end(), moreFiles.begin(), moreFiles.end());
                            }
                        } else { // pattern
                            if (auto const moreFiles {io::enumerate(io::get_parent_folder(f), {f, true}, false)}; !moreFiles.empty()) {
                                files.insert(files.end(), moreFiles.begin(), moreFiles.end());
                            } else {
                                logger::Error("texture asset '{}': File or folder '{}' not found.", asset->assetPtr.get()->get_name(), f);
                                continue;
                            }
                        }
                    }

                } else if (path file; assetSection.try_get(file, API::Texture::source)) {
                    path const f {mp + file};
                    if (io::is_file(f)) {
                        files.push_back(f);
                    } else {
                        logger::Error("texture asset '{}': File or folder '{}' not found.", asset->assetPtr.get()->get_name(), file);
                    }
                }
                for (u32 i {0}; i < files.size(); ++i) {
                    auto const& file {files[i]};
                    auto const  regionName {io::get_stem(file)};
                    asset->assetPtr->add_region(regionName, {{0.0f, 0.0f, 1.0f, 1.0f}, i});
                    asset->images.emplace_back(i, file);
                }

                if (object xyRegions; assetSection.try_get(xyRegions, API::Texture::xy_regions)) {
                    for (auto const& [regk, regv] : xyRegions) {
                        asset->abs_regions[regk] = regv.as<texture_region>();
                    }
                }
                if (object uvRegions; assetSection.try_get(uvRegions, API::Texture::uv_regions)) {
                    for (auto const& [regk, regv] : uvRegions) {
                        asset->assetPtr->add_region(regk, regv.as<texture_region>());
                    }
                }
                if (size_i size; assetSection.try_get(size, API::Texture::size)) {
                    asset->size = size;
                }
                if (texture::wrapping wrapping {}; assetSection.try_get(wrapping, API::Texture::wrapping)) {
                    asset->wrapping = wrapping;
                }
                if (texture::filtering filtering {}; assetSection.try_get(filtering, API::Texture::filtering)) {
                    asset->filtering = filtering;
                }
            }
            // texture strings
            else if (path assetString; v.try_get(assetString)) {
                path const f {mp + assetString};
                if (io::is_file(f)) {
                    auto const regionName {io::get_stem(f)};
                    asset->assetPtr->add_region(regionName, {{0.0f, 0.0f, 1.0f, 1.0f}, 0});
                    asset->images.emplace_back(0, f); //{.Depth = 0, .Path = file});
                }
            }
        }
    }

    if (object textureSection; _object.try_get(textureSection, API::AnimatedTexture::Name)) {
        for (auto const& [k, v] : textureSection) {
            auto* asset {default_new<animated_texture, ani_asset_def>(k, this, get_bucket(), _cacheAni)};
            asset->assetPtr->add_region("default", {{0.0f, 0.0f, 1.0f, 1.0f}, 0});

            if (object assetSection; v.try_get(assetSection)) {
                if (path source; assetSection.try_get(source, API::AnimatedTexture::source)) {
                    asset->textureFile = mp + source;
                }
                if (texture::wrapping wrapping {}; assetSection.try_get(wrapping, API::AnimatedTexture::wrapping)) {
                    asset->wrapping = wrapping;
                }
                if (texture::filtering filtering {}; assetSection.try_get(filtering, API::AnimatedTexture::filtering)) {
                    asset->filtering = filtering;
                }
            } else if (path assetString; v.try_get(assetString)) {
                asset->textureFile = mp + assetString;
            }
        }
    }
}

void cfg_texture_loader::prepare()
{
    for (auto const& def : _cacheTex) {
        auto const& name {def->assetPtr.get()->get_name()};

        if (def->images.empty()) {
            logger::Warning("texture asset '{}': No source files found.", name);
            continue;
        }

        auto& tex {*def->assetPtr};
        if (std::ssize(def->images) > locate_service<render_system>().get_capabilities().MaxArrayTextureLayers) {
            logger::Error("texture asset '{}': Layer count exceeds MaxArrayTextureLayers.", name);
            set_asset_status(def->assetPtr, status::Error);
            continue;
        }

        size_i    texSize {def->size};
        auto      texFormat {texture::format::RGBA8}; // TODO: api
        u32 const texDepth {static_cast<u32>(def->images.size())};

        if (texSize == size_i::Zero) {
            auto const& path {def->images.front().Path};
            auto        imgInfo {image::LoadInfo(path)};
            if (!imgInfo.has_value()) {
                logger::Error("texture asset '{}': Error loading image {}.", name, path);
                set_asset_status(def->assetPtr, status::Error);
                continue;
            }

            texSize = imgInfo->Size;

            switch (imgInfo->bytes_per_pixel()) {
            case 3:
                texFormat = texture::format::RGB8;
                break;
            case 4:
                texFormat = texture::format::RGBA8;
                break;
            default:
                continue;
            }
        }

        tex.create(texSize, texDepth, texFormat);

        tex.Filtering = def->filtering;
        tex.Wrapping  = def->wrapping;

        // convert region from pixels to relative
        auto const [w, h] {texSize};
        for (auto const& [k, v] : def->abs_regions) {
            tex.add_region(k, {{v.UVRect.left() / w, v.UVRect.top() / h, v.UVRect.Width / w, v.UVRect.Height / h}, v.Level});
        }

        // load images
        for (auto& img : def->images) {
            img.Future = img.Image.load_async(img.Path);

            auto& rel {_reloadInfoTex[def->assetPtr.get()->get_name()]};
            rel.push_back({.Depth = img.Depth, .Path = img.Path});
        }

        set_asset_status(def->assetPtr, status::Loading);
    }

    for (auto const& def : _cacheAni) {
        auto* ani {dynamic_cast<animated_texture*>(def->assetPtr.get_obj())};
        if (ani && ani->load(def->textureFile) == load_status::Ok) {
            ani->Filtering = def->filtering;
            ani->Wrapping  = def->wrapping;
            set_asset_status(def->assetPtr, status::Loaded);
        } else {
            set_asset_status(def->assetPtr, status::Error);
        }
    }
    _cacheAni.clear();

    locate_service<task_manager>().run_deferred([&]() { return check_async_load(); });
}

auto cfg_texture_loader::check_async_load() -> task_status
{
    if (_cacheTex.empty()) {
        return task_status::Finished;
    }

    // check if async images have been loaded and update textures
    bool loadingDone {true};
    for (auto it {_cacheTex.begin()}; it != _cacheTex.end(); ++it) {
        auto const& def {*it};
        if (def->assetPtr.get()->get_status() == status::Loaded) {
            continue;
        }
        auto& images {def->images};

        bool assetLoadingDone {true};
        for (auto imgIt {images.begin()}; imgIt != images.end(); ++imgIt) {
            if (auto& statusFuture {imgIt->Future}; statusFuture.valid()) {
                // not ready -> continue
                if (statusFuture.wait_for(0s) != std::future_status::ready) {
                    assetLoadingDone = false;
                    continue;
                }

                // loading error -> log and continue
                if (statusFuture.get() != load_status::Ok) {
                    logger::Error("texture asset '{}': Error loading image {}.", def->assetPtr.get()->get_name(), imgIt->Path);
                    set_asset_status(def->assetPtr, status::Error);
                    continue;
                }

                // update texture
                auto const& tex {*def->assetPtr};
                auto const& img {imgIt->Image};
                auto const& imgInfo {img.get_info()};

                if (tex.get_size() != imgInfo.Size) {
                    logger::Error("texture asset '{}': Error loading image {}.", def->assetPtr.get()->get_name(), imgIt->Path);
                    set_asset_status(def->assetPtr, status::Error);
                    continue;
                }

                tex.update_data(img.get_data(), imgIt->Depth, 0, imgInfo.bytes_per_pixel() == 4 ? 4 : 1);
                imgIt->Image = {};
            }
        }

        if (assetLoadingDone) {
            if (def->assetPtr.get()->get_status() != status::Error) {
                set_asset_status(def->assetPtr, status::Loaded);
            }
        } else {
            loadingDone = false;
        }
    }

    if (loadingDone) {
        _cacheTex.clear();
    }

    return _cacheTex.empty() ? task_status::Finished : task_status::Running;
}

}
