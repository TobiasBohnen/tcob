// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/assets/AssetLoader.hpp"

#include <memory>
#include <utility>

namespace tcob::assets {

////////////////////////////////////////////////////////////

void loader_manager::prepare()
{
    for (auto& loader : _loaders) {
        loader->prepare();
    }
}

void loader_manager::load()
{
    for (auto& loader : _loaders) {
        loader->load();
    }
}

void loader_manager::add_loader(std::unique_ptr<detail::loader_base> loader)
{
    _loaders.push_back(std::move(loader));
}

}
