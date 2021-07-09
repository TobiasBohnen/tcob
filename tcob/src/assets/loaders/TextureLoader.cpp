// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TextureLoader.hpp"

#include <tcob/core/io/FileSystem.hpp>
#include <tcob/gfx/gl/GLCapabilities.hpp>
#include <tcob/script/LuaScript.hpp>

using namespace std::chrono_literals;

namespace tcob::detail {

static const std::unordered_map<std::string, tcob::gl::TextureWrap> wrapping {
    { "ClampToEdge", tcob::gl::TextureWrap::ClampToEdge },
    { "ClampToBorder", tcob::gl::TextureWrap::ClampToBorder },
    { "MirroredRepeat", tcob::gl::TextureWrap::MirroredRepeat },
    { "Repeat", tcob::gl::TextureWrap::Repeat },
    { "MirrorClampToEdge", tcob::gl::TextureWrap::MirrorClampToEdge }
};
static const std::unordered_map<std::string, tcob::gl::TextureFiltering> filtering {
    { "Linear", tcob::gl::TextureFiltering::Linear },
    { "NearestNeighbor", tcob::gl::TextureFiltering::NearestNeighbor }
};

TextureLoader::TextureLoader(ResourceGroup& group)
    : ResourceLoader<gl::Texture> { group }
{
}

void TextureLoader::register_wrapper(lua::Script& script)
{
    // texture
    _funcNewTexture = [this](const std::string& s) {
        auto texture { get_or_create_resource<gl::Texture2D>(s) };
        auto def { std::make_unique<TextureDef>() };
        def->name = s;
        def->Res = texture;

        auto retValue { def.get() };
        _textureCache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["texture"] = _funcNewTexture;

    auto& texturewrap { script.create_wrapper<TextureDef>("TextureDef") };
    texturewrap.function("source", [this](TextureDef* def, const std::string& textureFile) {
        std::string f { group().mount_point() + textureFile };

        auto tex { dynamic_cast<gl::Texture2D*>(def->Res.object()) };
        tex->regions()[def->name] = { { 0.f, 0.f, 1.f, 1.f }, 0 };
        tex->regions()["default"] = { { 0.f, 0.f, 1.f, 1.f }, 0 };
        def->textureFile = f;

        return def;
    });
    texturewrap.function("regions", [](TextureDef* def, const std::unordered_map<std::string, RectF>& table) {
        for (auto& [k, v] : table) {
            def->regions[k] = v;
        }
        return def;
    });
    texturewrap.function("wrapping", [](TextureDef* def, const std::string& wrap) {
        if (wrapping.contains(wrap))
            def->wrapping = wrapping.at(wrap);
        return def;
    });
    texturewrap.function("filtering", [](TextureDef* def, const std::string& filter) {
        if (filtering.contains(filter))
            def->filtering = filtering.at(filter);
        return def;
    });

    // texture array
    _funcNewTextureArray = [this](const std::string& s) {
        auto texture { get_or_create_resource<gl::Texture2DArray>(s) };
        auto def { std::make_unique<TextureArrayDef>() };
        def->name = s;
        def->Res = texture;

        auto retValue { def.get() };
        _textureArrayCache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["texture_array"] = _funcNewTextureArray;

    auto& texturearraywrap { script.create_wrapper<TextureArrayDef>("TextureArrayDef") };
    texturearraywrap.function("source", [this](TextureArrayDef* def, const std::vector<std::string>& items) {
        // texture array
        std::vector<std::string> files;
        for (auto& item : items) {
            std::string f { group().mount_point() + item };

            if (!FileSystem::exists(f)) {
                //TODO: log error
                continue;
            }

            if (FileSystem::is_file(f)) {
                files.push_back(f);
            } else if (FileSystem::is_folder(f)) {
                std::vector<std::string> moreFiles { FileSystem::enumerate(f) };
                files.insert(files.end(), moreFiles.begin(), moreFiles.end());
            }
        }

        def->Res->regions()["default"] = { { 0.f, 0.f, 1.f, 1.f }, 0 };

        for (u32 i { 0 }; i < files.size(); ++i) {
            const auto& file { files[i] };
            auto regionName { FileSystem::stem(file) };
            def->Res->regions()[regionName] = { { 0.f, 0.f, 1.f, 1.f }, i };
            def->textureFiles[i] = file;
        }

        return def;
    });
    texturearraywrap.function("wrapping", [](TextureArrayDef* def, const std::string& wrap) {
        if (wrapping.contains(wrap))
            def->wrapping = wrapping.at(wrap);
        return def;
    });
    texturearraywrap.function("filtering", [](TextureArrayDef* def, const std::string& filter) {
        if (filtering.contains(filter))
            def->filtering = filtering.at(filter);
        return def;
    });
}

void TextureLoader::on_updating()
{
    async_load();
}

void TextureLoader::on_preparing()
{
    if (!_textureCache.empty()) {
        for (auto& def : _textureCache) {

            auto tex2d { dynamic_cast<gl::Texture2D*>(def->Res.object()) };
            tex2d->create_or_resize(Image::Info(def->textureFile).SizeInPixels);
            tex2d->wrapping(def->wrapping);
            tex2d->filtering(def->filtering);

            auto [w, h] { tex2d->size() };
            for (auto& [k, v] : def->regions) {
                tex2d->regions()[k] = { { v.Left / w, v.Top / h, v.Width / w, v.Height / h }, 0 };
            }

            def->ImageFtr = Image::LoadAsync(def->textureFile);
        }
    }

    if (!_textureArrayCache.empty()) {
        for (auto& def : _textureArrayCache) {
            if (def->textureFiles.empty()) {
                // TODO: log error
                continue;
            }

            auto tex2darray { dynamic_cast<gl::Texture2DArray*>(def->Res.object()) };
            if (def->textureFiles.size() > gl::Capabilities::MaxArrayTextureLayers) {
                // TODO: log error
                continue;
            }

            tex2darray->create_or_resize(Image::Info(def->textureFiles[0]).SizeInPixels, static_cast<u32>(def->textureFiles.size()));
            tex2darray->wrapping(def->wrapping);
            tex2darray->filtering(def->filtering);

            for (auto& [level, file] : def->textureFiles) {
                def->ImageFtrs[level] = Image::LoadAsync(file);
            }
        }
    }
}

auto TextureLoader::async_load() -> bool
{
    if (_textureCache.empty() && _textureArrayCache.empty()) {
        return true;
    }

    // check if async images have been loaded and update textures
    for (auto it { _textureCache.begin() }; it != _textureCache.end();) {
        auto& def { *it };
        auto& fut { def->ImageFtr };

        if (fut.wait_for(0s) == std::future_status::ready) {
            auto img { fut.get() };
            dynamic_cast<gl::Texture2D*>(def->Res.object())->update(PointU::Zero, img.info().SizeInPixels, img.buffer(), 0, 4);
            set_resource_loaded(def->Res);
            it = _textureCache.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it { _textureArrayCache.begin() }; it != _textureArrayCache.end();) {
        auto& def { *it };
        auto& futs { def->ImageFtrs };

        for (auto futit { futs.begin() }; futit != futs.end();) {
            auto& fut { (*futit).second };
            if (fut.wait_for(0s) == std::future_status::ready) {
                auto img { fut.get() };
                dynamic_cast<gl::Texture2DArray*>(def->Res.object())->update(PointU::Zero, img.info().SizeInPixels, img.buffer(), (*futit).first, 0, 4);
                futit = futs.erase(futit);
            } else {
                ++futit;
            }
        }

        if (futs.empty()) {
            set_resource_loaded(def->Res);
            it = _textureArrayCache.erase(it);
        }
    }

    return false;
}
}