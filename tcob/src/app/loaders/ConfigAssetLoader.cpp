// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigAssetLoader.hpp"

#include <any>
#include <chrono>
#include <future>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "tcob/audio/Buffer.hpp"
#include "tcob/audio/Music.hpp"
#include "tcob/audio/Sound.hpp"
#include "tcob/audio/synth/SoundFont.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/core/assets/AssetLoader.hpp"
#include "tcob/core/assets/Assets.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/FontFamily.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/animation/Animation.hpp"
#include "tcob/gfx/drawables/Cursor.hpp"

using namespace tcob::gfx;
using namespace tcob::audio;
using namespace tcob::data;
using namespace tcob::assets;

using namespace std::chrono_literals;

namespace io = tcob::io;

namespace tcob::API {
namespace SpriteAnimation {
    static char const* Name {"sprite_animation"};
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

namespace AudioBuffer {
    static char const* Name {"audio_buffer"};
    static char const* source {"source"};
}

namespace SoundFont {
    static char const* Name {"sound_font"};
    static char const* source {"source"};
}

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

    static char const* passes {"passes"};

    static char const* texture {"texture"};
    static char const* shader {"shader"};

    static char const* blend_func {"blend_func"};
    static char const* separate_blend_func {"separate_blend_func"};
    static char const* blend_equation {"blend_equation"};

    static char const* color {"color"};

    static char const* stencil_func {"stencil_func"};
    static char const* stencil_op {"stencil_op"};
    static char const* stencil_ref {"stencil_ref"};
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

template <typename T, typename TDef>
auto default_create(string const& name, auto&& group, auto&& cache) -> TDef&
{
    TDef& def {cache.emplace_back()};
    def.assetPtr = group.template create<T>(name);
    return def;
}

void default_check_async_load(def_task const& ctx, auto&& cache, auto&& stateSetter)
{
    using namespace std::chrono_literals;

    if (cache.empty()) {
        ctx.Finished = true;
        return;
    }

    bool loadingDone {true};

    for (auto& def : cache) {
        auto& ftr {def.future};
        if (!ftr.valid()) { continue; }

        if (ftr.wait_for(0s) == std::future_status::ready) {
            if (ftr.get()) {
                stateSetter(def.assetPtr, asset_status::Loaded);
            } else {
                stateSetter(def.assetPtr, asset_status::Error);
            }
        } else {
            loadingDone = false;
        }
    }

    if (loadingDone) { cache.clear(); }

    ctx.Finished = cache.empty();
}

template <typename TDef>
void try_get_source(data::entry const& v, TDef& asset, string_view key)
{
    if (data::object assetSection; v.try_get(assetSection)) {
        assetSection.try_get(asset.source, key);
    } else if (path assetString; v.try_get(assetString)) {
        asset.source = assetString;
    }
}

cfg_asset_loader_manager::cfg_asset_loader_manager(group& group)
{
    add_loader(std::make_unique<cfg_shader_loader>(group, _object));
    add_loader(std::make_unique<cfg_texture_loader>(group, _object));
    add_loader(std::make_unique<cfg_material_loader>(group, _object));
    add_loader(std::make_unique<cfg_cursor_loader>(group, _object));
    add_loader(std::make_unique<cfg_font_loader>(group, _object));
    add_loader(std::make_unique<cfg_font_family_loader>(group, _object));
    add_loader(std::make_unique<cfg_animation_loader>(group, _object));
    add_loader(std::make_unique<cfg_music_loader>(group, _object));
    add_loader(std::make_unique<cfg_sound_loader>(group, _object));
    add_loader(std::make_unique<cfg_audio_buffer_loader>(group, _object));
    add_loader(std::make_unique<cfg_sound_font_loader>(group, _object));
}

void cfg_asset_loader_manager::load(path const& file)
{
    object load;
    if (load.load(file)) {
        _object.merge(load, true);
    } else {
        logger::Error("cfg_asset_loader_manager '{}': file loading failed", file);
    }
}

////////////////////////////////////////////////////////////

cfg_animation_loader::cfg_animation_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_animation_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::SpriteAnimation::Name)) { return; }

    for (auto const& [k, v] : obj) {
        if (object assetSection; v.try_get(assetSection)) {
            std::vector<sprite_animation::frame> frames;
            assetSection.try_get(frames, API::SpriteAnimation::frames);

            asset_def& def {_cache.emplace_back()};
            def.assetPtr = group().create<sprite_animation>(k, frames);
        }
    }
}

void cfg_animation_loader::prepare()
{
    for (auto& def : _cache) {
        set_asset_status(def.assetPtr, asset_status::Loaded);
    }
    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_music_loader::cfg_music_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_music_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Music::Name)) { return; }

    for (auto const& [k, v] : obj) {
        auto& def {default_create<music, asset_def>(k, group(), _cache)};
        try_get_source(v, def, API::Music::source);
    }
}

void cfg_music_loader::prepare()
{
    for (auto& def : _cache) {
        if (def.assetPtr->open(group().mount_point() + def.source)) {
            set_asset_status(def.assetPtr, asset_status::Loaded);
        } else {
            set_asset_status(def.assetPtr, asset_status::Error);
        }
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_sound_loader::cfg_sound_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_sound_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Sound::Name)) { return; }

    for (auto const& [k, v] : obj) {
        auto& def {default_create<sound, asset_def>(k, group(), _cache)};
        try_get_source(v, def, API::Sound::source);
    }
}

void cfg_sound_loader::prepare()
{
    for (auto& def : _cache) {
        def.future = def.assetPtr->load_async(group().mount_point() + def.source);
        set_asset_status(def.assetPtr, asset_status::Loading);
    }

    locate_service<task_manager>().run_deferred([this](def_task const& ctx) {
        default_check_async_load(ctx, _cache, [this](auto&& asset, auto&& state) { set_asset_status(asset, state); });
    });
}

////////////////////////////////////////////////////////////

cfg_audio_buffer_loader::cfg_audio_buffer_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_audio_buffer_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::AudioBuffer::Name)) { return; }

    for (auto const& [k, v] : obj) {
        auto& def {default_create<audio::buffer, asset_def>(k, group(), _cache)};
        try_get_source(v, def, API::AudioBuffer::source);
    }
}

void cfg_audio_buffer_loader::prepare()
{
    std::any a {}; // DUMMY
    for (auto& def : _cache) {
        def.future = def.assetPtr->load_async(group().mount_point() + def.source, a);
        set_asset_status(def.assetPtr, asset_status::Loading);
    }

    locate_service<task_manager>().run_deferred([this](def_task const& ctx) {
        default_check_async_load(ctx, _cache, [this](auto&& asset, auto&& state) { set_asset_status(asset, state); });
    });
}

////////////////////////////////////////////////////////////

cfg_sound_font_loader::cfg_sound_font_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_sound_font_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::SoundFont::Name)) { return; }

    for (auto const& [k, v] : obj) {
        auto& def {default_create<sound_font, asset_def>(k, group(), _cache)};
        try_get_source(v, def, API::SoundFont::source);
    }
}

void cfg_sound_font_loader::prepare()
{
    for (auto& def : _cache) {
        def.future = def.assetPtr->load_async(group().mount_point() + def.source);
        set_asset_status(def.assetPtr, asset_status::Loading);
    }

    locate_service<task_manager>().run_deferred([this](def_task const& ctx) {
        default_check_async_load(ctx, _cache, [this](auto&& asset, auto&& state) { set_asset_status(asset, state); });
    });
}

////////////////////////////////////////////////////////////

cfg_cursor_loader::cfg_cursor_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_cursor_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Cursor::Name)) { return; }

    for (auto const& [k, v] : obj) {
        if (object assetSection; v.try_get(assetSection)) {
            auto& def {default_create<cursor, asset_def>(k, group(), _cache)};
            assetSection.try_get(def.material, API::Cursor::material);

            if (object modesSection; assetSection.try_get(modesSection, API::Cursor::modes)) {
                for (auto const& [mk, mv] : modesSection) {
                    if (object modeSection; mv.try_get(modeSection)) {
                        def.assetPtr->add_mode(mk, modeSection["hotspot"].as<point_i>());
                    }
                }
            }
        }
    }
}

void cfg_cursor_loader::prepare()
{
    for (auto& def : _cache) {
        if (!group().has<material>(def.material)) {
            set_asset_status(def.assetPtr, asset_status::Error);
            continue;
        }

        def.assetPtr->Material = group().get<material>(def.material);
        set_asset_status(def.assetPtr, asset_status::Loaded);
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_font_loader::cfg_font_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_font_loader::declare()
{
    if (object fontSection; _object.try_get(fontSection, API::TrueTypeFont::Name)) {
        for (auto const& [k, v] : fontSection) {
            if (object assetSection; v.try_get(assetSection)) {
                auto& def {default_create<font, asset_def>(k, group(), _cache)};
                assetSection.try_get(def.source, API::TrueTypeFont::source);
                assetSection.try_get(def.size, API::TrueTypeFont::size);
            }
        }
    }
}

void cfg_font_loader::prepare()
{
    for (auto& def : _cache) {
        auto* ttf {def.assetPtr.ptr()};
        if (ttf->load(group().mount_point() + def.source, def.size)) {
            set_asset_status(def.assetPtr, asset_status::Loaded);
        } else {
            set_asset_status(def.assetPtr, asset_status::Error);
        }
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_font_family_loader::cfg_font_family_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_font_family_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::FontFamily::Name)) { return; }

    for (auto const& [k, v] : obj) {
        asset_def& def {_cache.emplace_back()};
        def.assetPtr = group().create<font_family>(k, k);
        try_get_source(v, def, API::FontFamily::source);
    }
}

void cfg_font_family_loader::prepare()
{
    auto& grp {group()};

    for (auto const& def : _cache) {
        font_family::FindSources(*def.assetPtr, grp.mount_point() + def.source);
        set_asset_status(def.assetPtr, asset_status::Loaded);
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_material_loader::cfg_material_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_material_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Material::Name)) { return; }

    auto const getPass {[](object const& assetSection, pass_def& def) {
        assetSection.try_get(def.texture, API::Material::texture);
        assetSection.try_get(def.shader, API::Material::shader);

        if (object blendFunc; assetSection.try_get(blendFunc, API::Material::blend_func)) {
            blend_func s {blendFunc["source"].as<blend_func>()};
            def.pass.BlendFuncs.SourceAlphaBlendFunc = s;
            def.pass.BlendFuncs.SourceColorBlendFunc = s;

            blend_func d {blendFunc["destination"].as<blend_func>()};
            def.pass.BlendFuncs.DestinationAlphaBlendFunc = d;
            def.pass.BlendFuncs.DestinationColorBlendFunc = d;
        } else {
            assetSection.try_get(def.pass.BlendFuncs, API::Material::separate_blend_func);
        }

        assetSection.try_get(def.pass.BlendEquation, API::Material::blend_equation);
        assetSection.try_get(def.pass.Color, API::Material::color);
        assetSection.try_get(def.pass.StencilFunc, API::Material::stencil_func);
        assetSection.try_get(def.pass.StencilOp, API::Material::stencil_op);
        assetSection.try_get(def.pass.StencilRef, API::Material::stencil_ref);
    }};

    for (auto const& [k, v] : obj) {
        object assetSection;
        if (!v.try_get(assetSection)) { continue; }

        auto& def {default_create<material, asset_def>(k, group(), _cache)};
        if (array passes; assetSection.try_get(passes, API::Material::passes)) {
            for (auto const& pass : passes) {
                if (object passObj; pass.try_get(passObj)) {
                    getPass(passObj, def.passes.emplace_back());
                }
            }
        } else {
            getPass(assetSection, def.passes.emplace_back());
        }
    }
}

void cfg_material_loader::prepare()
{
    auto& grp {group()};

    for (auto& def : _cache) {
        auto const& name {def.assetPtr.get()->name()};

        bool error {false};
        for (auto& passDef : def.passes) {
            if (!passDef.shader.empty()) {
                if (!grp.has<shader>(passDef.shader)) {
                    logger::Error("material asset '{}': Shader '{}' not found", name, passDef.shader);
                    set_asset_status(def.assetPtr, asset_status::Error);
                    error = true;
                    break;
                }

                passDef.pass.Shader = grp.get<shader>(passDef.shader);
            }

            if (!passDef.texture.empty()) {
                if (!grp.has<texture>(passDef.texture)) {
                    logger::Error("material asset '{}': Texture '{}' not found", name, passDef.texture);
                    set_asset_status(def.assetPtr, asset_status::Error);
                    error = true;
                    break;
                }

                passDef.pass.Texture = grp.get<texture>(passDef.texture);
            }

            def.assetPtr->create_pass() = passDef.pass;
        }

        if (!error) {
            set_asset_status(def.assetPtr, asset_status::Loaded);
        }
    }

    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_shader_loader::cfg_shader_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_shader_loader::declare()
{
    object obj;
    if (!_object.try_get(obj, API::Shader::Name)) { return; }

    for (auto const& [k, v] : obj) {
        if (object assetSection; v.try_get(assetSection)) {
            // try getting render system specific shader
            assetSection.try_get(assetSection, locate_service<render_system>().name());

            asset_def def {};

            assetSection.try_get(def.vertex, API::Shader::vertex);
            auto const vertSource {io::read_as_string(group().mount_point() + def.vertex)};
            if (vertSource.empty()) {
                logger::Error("shader asset '{}': Vertex shader '{}' not found", k, def.vertex);
                continue;
            }

            assetSection.try_get(def.fragment, API::Shader::fragment);
            auto const fragSource {io::read_as_string(group().mount_point() + def.fragment)};
            if (fragSource.empty()) {
                logger::Error("shader asset '{}': Fragment shader '{}' not found", k, def.fragment);
                continue;
            }

            def.assetPtr = group().create<gfx::shader>(k, vertSource, fragSource);
            _cache.push_back(std::move(def));
        }
    }
}

void cfg_shader_loader::prepare()
{
    for (auto& def : _cache) {
        set_asset_status(def.assetPtr, def.assetPtr->is_valid() ? asset_status::Loaded : asset_status::Error);
    }
    _cache.clear();
}

////////////////////////////////////////////////////////////

cfg_texture_loader::cfg_texture_loader(assets::group& group, data::object& object)
    : loader {group}
    , _object {object}
{
}

void cfg_texture_loader::declare()
{
    path const mp {group().mount_point()};

    if (object textureSection; _object.try_get(textureSection, API::Texture::Name)) {
        for (auto const& [k, v] : textureSection) {
            auto& def {default_create<texture, tex_asset_def>(k, group(), _cacheTex)};
            def.assetPtr->regions()["default"] = {.UVRect = {0.0f, 0.0f, 1.0f, 1.0f}, .Level = 0};

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
                                auto const moreFiles {io::enumerate(f, {.String = "*.*", .MatchWholePath = true}, false)};
                                files.insert(files.end(), moreFiles.begin(), moreFiles.end());
                            }
                        } else { // pattern
                            if (auto const moreFiles {io::enumerate(io::get_parent_folder(f), {.String = f, .MatchWholePath = true}, false)}; !moreFiles.empty()) {
                                files.insert(files.end(), moreFiles.begin(), moreFiles.end());
                            } else {
                                logger::Error("texture asset '{}': File or folder '{}' not found", k, f);
                                continue;
                            }
                        }
                    }
                } else if (path file; assetSection.try_get(file, API::Texture::source)) {
                    path const f {mp + file};
                    if (io::is_file(f)) {
                        files.push_back(f);
                    } else {
                        logger::Error("texture asset '{}': File or folder '{}' not found", k, file);
                    }
                }

                for (u32 i {0}; i < files.size(); ++i) {
                    auto const& file {files[i]};
                    auto const  regionName {io::get_stem(file)};
                    def.assetPtr->regions()[regionName] = {.UVRect = {0.0f, 0.0f, 1.0f, 1.0f}, .Level = i};
                    def.imageFtrs.emplace_back(i, file);
                }

                if (object xyRegions; assetSection.try_get(xyRegions, API::Texture::xy_regions)) {
                    for (auto const& [regk, regv] : xyRegions) {
                        def.abs_regions[regk] = regv.as<texture_region>();
                    }
                }
                if (object uvRegions; assetSection.try_get(uvRegions, API::Texture::uv_regions)) {
                    for (auto const& [regk, regv] : uvRegions) {
                        def.assetPtr->regions()[regk] = regv.as<texture_region>();
                    }
                }
                assetSection.try_get(def.size, API::Texture::size);
                assetSection.try_get(def.wrapping, API::Texture::wrapping);
                assetSection.try_get(def.filtering, API::Texture::filtering);
            }
            // texture strings
            else if (path assetString; v.try_get(assetString)) {
                path const f {mp + assetString};
                if (io::is_file(f)) {
                    auto const regionName {io::get_stem(f)};
                    def.assetPtr->regions()[regionName] = {.UVRect = {0.0f, 0.0f, 1.0f, 1.0f}, .Level = 0};
                    def.imageFtrs.emplace_back(0, f);
                }
            }
        }
    }

    if (object textureSection; _object.try_get(textureSection, API::AnimatedTexture::Name)) {
        for (auto const& [k, v] : textureSection) {
            auto& def {default_create<animated_texture, ani_asset_def>(k, group(), _cacheAni)};
            def.assetPtr->regions()["default"] = {.UVRect = {0.0f, 0.0f, 1.0f, 1.0f}, .Level = 0};

            if (object assetSection; v.try_get(assetSection)) {
                if (path source; assetSection.try_get(source, API::AnimatedTexture::source)) {
                    def.textureFile = mp + source;
                }
                assetSection.try_get(def.wrapping, API::AnimatedTexture::wrapping);
                assetSection.try_get(def.filtering, API::AnimatedTexture::filtering);
            } else if (path assetString; v.try_get(assetString)) {
                def.textureFile = mp + assetString;
            }
        }
    }
}

void cfg_texture_loader::prepare()
{
    for (auto& def : _cacheTex) {
        auto const& name {def.assetPtr.get()->name()};

        if (def.imageFtrs.empty()) {
            logger::Warning("texture asset '{}': No source files found", name);
            continue;
        }

        auto& tex {*def.assetPtr};
        if (std::ssize(def.imageFtrs) > locate_service<render_system>().info().Texture.MaxLayers) {
            logger::Error("texture asset '{}': Layer count exceeds MaxArrayTextureLayers", name);
            set_asset_status(def.assetPtr, asset_status::Error);
            continue;
        }

        size_i    texSize {def.size};
        auto      texFormat {texture::format::RGBA8}; // TODO: api
        u32 const texDepth {static_cast<u32>(def.imageFtrs.size())};

        if (texSize == size_i::Zero) {
            auto const& path {def.imageFtrs.front().path};
            auto        imgInfo {image::LoadInfo(path)};
            if (!imgInfo.has_value()) {
                logger::Error("texture asset '{}': Error loading image {}", name, path);
                set_asset_status(def.assetPtr, asset_status::Error);
                continue;
            }

            texSize = imgInfo->Size;

            switch (imgInfo->bytes_per_pixel()) {
            case 3:  texFormat = texture::format::RGB8; break;
            case 4:  texFormat = texture::format::RGBA8; break;
            default: set_asset_status(def.assetPtr, asset_status::Error); continue;
            }
        }

        tex.resize(texSize, texDepth, texFormat);

        tex.Filtering = def.filtering;
        tex.Wrapping  = def.wrapping;

        // convert region from pixels to relative
        auto const [w, h] {size_f {texSize}};
        for (auto const& [k, v] : def.abs_regions) {
            tex.regions()[k] = {.UVRect = {v.UVRect.left() / w, v.UVRect.top() / h, v.UVRect.width() / w, v.UVRect.height() / h}, .Level = v.Level};
        }

        // load images
        for (auto& img : def.imageFtrs) {
            img.future = img.image.load_async(img.path);
        }

        set_asset_status(def.assetPtr, asset_status::Loading);
    }

    for (auto& def : _cacheAni) {
        auto* ani {dynamic_cast<animated_texture*>(def.assetPtr.ptr())};
        if (ani && ani->load(def.textureFile)) {
            ani->Filtering = def.filtering;
            ani->Wrapping  = def.wrapping;
            set_asset_status(def.assetPtr, asset_status::Loaded);
        } else {
            set_asset_status(def.assetPtr, asset_status::Error);
        }
    }
    _cacheAni.clear();

    locate_service<task_manager>().run_deferred([this](def_task const& ctx) { check_async_load(ctx); });
}

void cfg_texture_loader::check_async_load(def_task const& ctx)
{
    if (_cacheTex.empty()) {
        ctx.Finished = true;
        return;
    }

    // check if async images have been loaded and update textures
    bool loadingDone {true};
    for (auto& def : _cacheTex) {
        if (def.assetPtr.get()->status() == asset_status::Loaded) { continue; }

        bool assetLoadingDone {true};
        for (auto& imageFtr : def.imageFtrs) {
            if (auto& statusFuture {imageFtr.future}; statusFuture.valid()) {
                // not ready -> continue
                if (statusFuture.wait_for(0s) != std::future_status::ready) {
                    assetLoadingDone = false;
                    continue;
                }

                // loading error -> log and continue
                if (!statusFuture.get()) {
                    logger::Error("texture asset '{}': Error loading image {}", def.assetPtr.get()->name(), imageFtr.path);
                    set_asset_status(def.assetPtr, asset_status::Error);
                    continue;
                }

                // update texture
                auto const& tex {*def.assetPtr};
                auto const& img {imageFtr.image};
                auto const& imgInfo {img.info()};

                if (tex.info().Size != imgInfo.Size) {
                    logger::Error("texture asset '{}': Error loading image {}", def.assetPtr.get()->name(), imageFtr.path);
                    set_asset_status(def.assetPtr, asset_status::Error);
                    continue;
                }

                tex.update_data(img.data(), imageFtr.depth, 0, imgInfo.bytes_per_pixel() == 4 ? 4 : 1);
                imageFtr.image = {};
            }
        }

        if (assetLoadingDone) {
            if (def.assetPtr.get()->status() != asset_status::Error) {
                set_asset_status(def.assetPtr, asset_status::Loaded);
            }
        } else {
            loadingDone = false;
        }
    }

    if (loadingDone) {
        _cacheTex.clear();
    }

    ctx.Finished = _cacheTex.empty();
}
}
