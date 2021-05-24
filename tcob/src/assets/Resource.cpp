// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/assets/Resource.hpp>

#include <tcob/assets/ResourceLibrary.hpp>

namespace tcob {
ResourceLoaderBase::ResourceLoaderBase(ResourceGroup& group)
    : _group { group }
{
    group.Loading.connect(&ResourceLoaderBase::on_loading, this);
    group.Preparing.connect(&ResourceLoaderBase::on_preparing, this);
    group.Unloading.connect(&ResourceLoaderBase::on_unloading, this);
    group.Updating.connect(&ResourceLoaderBase::on_updating, this);
}

auto ResourceLoaderBase::group() -> ResourceGroup&
{
    return _group;
}
}