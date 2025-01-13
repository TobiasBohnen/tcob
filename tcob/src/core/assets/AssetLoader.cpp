// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/assets/AssetLoader.hpp"

namespace tcob::assets {

////////////////////////////////////////////////////////////

void loader_manager::declare()
{
    for (auto& loader : _loaders) {
        loader->declare();
    }
}

void loader_manager::prepare()
{
    for (auto& loader : _loaders) {
        loader->prepare();
    }
}

void loader_manager::add_loader(std::unique_ptr<detail::loader_base> loader)
{
    _loaders.push_back(std::move(loader));
}

}
