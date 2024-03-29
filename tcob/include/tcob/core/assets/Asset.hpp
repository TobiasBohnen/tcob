// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cstddef>
#include <memory>

#include "tcob/core/Concepts.hpp"

namespace tcob::assets {
////////////////////////////////////////////////////////////

template <typename T>
class asset;

template <typename T>
class asset_ptr;

template <typename T>
class bucket;

class group;

class library;

template <typename T>
class loader;

class loader_manager;

////////////////////////////////////////////////////////////

enum class status : u8 {
    Unloaded,
    Created,
    Loading,
    Loaded,
    Error
};

////////////////////////////////////////////////////////////

template <typename T>
class asset {
    template <typename U>
    friend class asset;
    template <typename U>
    friend class loader;

public:
    using type = T;

    asset(string name, std::weak_ptr<T> ptr, loader<T>* loader);
    template <BaseOrDerivedOf<T> U>
    asset(asset<U> const& other) noexcept;

    template <BaseOrDerivedOf<T> U>
    auto operator=(asset<U> const& other) noexcept -> asset<T>&;

    auto operator->() const -> type*;
    auto operator*() const -> type&;

    auto get_name() const -> string const&;
    auto get_status() const -> status;

    auto get() const -> type*;
    auto get_ptr() const -> std::shared_ptr<T>;

    auto reset(std::weak_ptr<T> ptr);

    void unload(bool greedy = false);
    auto reload() -> bool;

    explicit operator bool() const;
    auto     is_expired() const -> bool;
    auto     is_ready() const -> bool;

private:
    void set_status(status status);

    string           _name {};
    std::weak_ptr<T> _object {};
    loader<T>*       _loader {nullptr};
    status           _status {status::Error};
};

////////////////////////////////////////////////////////////

template <typename T>
class asset_ptr {
    template <typename U>
    friend class asset_ptr;

public:
    using type = T;

    asset_ptr() = default;
    asset_ptr(std::nullptr_t);
    explicit asset_ptr(std::shared_ptr<asset<T>> ptr);
    template <BaseOrDerivedOf<T> U>
    asset_ptr(asset_ptr<U> const& other) noexcept;

    template <BaseOrDerivedOf<T> U>
    auto operator=(asset_ptr<U> const& other) noexcept -> asset_ptr<T>&;

    auto operator->() const -> type*;
    auto operator*() const -> type&;

    auto get() const -> asset<T>*;
    auto get_ptr() const -> std::shared_ptr<T>;
    auto get_obj() const -> T*;

    auto get_use_count() const -> i32;

    explicit operator bool() const;
    auto     is_expired() const -> bool;
    auto     is_ready() const -> bool;

    void reset();

private:
    std::shared_ptr<asset<T>> _asset;
};

template <typename T>
inline auto operator==(asset_ptr<T> const& left, asset_ptr<T> const& right) -> bool;

////////////////////////////////////////////////////////////

template <typename T>
class manual_asset_ptr {
public:
    using type = T;

    manual_asset_ptr(string const& name = "");

    auto operator->() const -> type*;
    auto operator*() const -> type&;
    template <BaseOrDerivedOf<T> U>
    operator asset_ptr<U>() const;

    auto get_obj() const -> type*;

    void reset();

private:
    std::shared_ptr<T> _object;
    asset_ptr<T>       _assetPtr;
};

}

#include "Asset.inl"
