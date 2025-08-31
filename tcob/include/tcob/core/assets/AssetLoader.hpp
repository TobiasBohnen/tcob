// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/core/assets/Assets.hpp"

namespace tcob::assets {
////////////////////////////////////////////////////////////

namespace detail {
    class loader_base {
    public:
        virtual ~loader_base() = default;

        virtual void declare() { }
        virtual void prepare() { }
    };
}

////////////////////////////////////////////////////////////

template <typename T>
class loader : public detail::loader_base {
public:
    explicit loader(assets::group& group);

protected:
    auto group() -> assets::group&;
    auto bucket() -> assets::bucket<T>*;

    void set_asset_status(asset_ptr<T> asset, asset_status status);

private:
    assets::group& _group;
};
////////////////////////////////////////////////////////////

class TCOB_API loader_manager : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<loader_manager>, group&> {
        static inline char const* ServiceName {"assets::loader_manager::factory"};
    };

    loader_manager()          = default;
    virtual ~loader_manager() = default;

    virtual void load_script(path const& file) = 0;
    void         declare();
    void         prepare();

protected:
    void add_loader(std::unique_ptr<detail::loader_base> loader);

private:
    std::vector<std::unique_ptr<detail::loader_base>> _loaders;
};

}

#include "AssetLoader.inl"
