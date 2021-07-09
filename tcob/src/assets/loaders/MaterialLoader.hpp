// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/Material.hpp>

namespace tcob::detail {
class MaterialLoader : public ResourceLoader<Material> {
public:
    explicit MaterialLoader(ResourceGroup& group);

    void register_wrapper(lua::Script& script) override;

protected:
    void on_preparing() override;
    void do_unload(ResourcePtr<Material> res, bool greedy) override;

private:
    struct MaterialDef {
        ResourcePtr<Material> Res;
        std::string shader;
        std::string texture;
    };

    std::function<MaterialDef*(const std::string&)> _funcNew;
    std::vector<std::unique_ptr<MaterialDef>> _cache;
};
}