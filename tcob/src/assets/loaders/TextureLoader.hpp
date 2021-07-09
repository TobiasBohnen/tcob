// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob::detail {
class TextureLoader : public ResourceLoader<gl::Texture> {
public:
    explicit TextureLoader(ResourceGroup& group);

    void register_wrapper(lua::Script& script) override;

protected:
    void on_preparing() override;
    void on_updating() override;

private:
    auto async_load() -> bool;

    struct TextureDef {
        ResourcePtr<gl::Texture> Res;
        std::string name;
        std::unordered_map<std::string, RectF> regions;
        std::string textureFile;
        gl::TextureFiltering filtering { gl::TextureFiltering::NearestNeighbor };
        gl::TextureWrap wrapping { gl::TextureWrap::Repeat };
        std::future<Image> ImageFtr;
    };

    std::function<TextureDef*(const std::string&)> _funcNewTexture;
    std::vector<std::unique_ptr<TextureDef>> _textureCache;

    struct TextureArrayDef {
        ResourcePtr<gl::Texture> Res;
        std::string name;
        gl::TextureFiltering filtering { gl::TextureFiltering::NearestNeighbor };
        gl::TextureWrap wrapping { gl::TextureWrap::Repeat };
        std::unordered_map<u32, std::string> textureFiles;
        std::unordered_map<u32, std::future<Image>> ImageFtrs;
    };

    std::function<TextureArrayDef*(const std::string&)> _funcNewTextureArray;
    std::vector<std::unique_ptr<TextureArrayDef>> _textureArrayCache;
};
}