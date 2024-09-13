// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Asset.hpp"

#include "tcob/core/Logger.hpp"

namespace tcob::assets {

template <typename T>
inline asset<T>::asset(string name, std::weak_ptr<T> ptr, loader<T>* loader)
    : _name {std::move(name)}
    , _object {std::move(ptr)}
    , _loader {loader}
{
    set_status(_loader ? status::Created : status::Loaded);
}

template <typename T>
template <BaseOfOrDerivedFrom<T> U>
inline asset<T>::asset(asset<U> const& other) noexcept
{
    *this = other;
}

template <typename T>
template <BaseOfOrDerivedFrom<T> U>
inline auto asset<T>::operator=(asset<U> const& other) noexcept -> asset<T>&
{
    _name   = other._name;
    _status = other._status;
    if (other._object.expired()) {
        return *this;
    }

    _object = std::dynamic_pointer_cast<T>(other._object.lock());
    if (_object.expired()) {
        logger::Error("asset: conversion failed: {}", _name);
    }

    return *this;
}

template <typename T>
inline auto asset<T>::operator->() const -> type*
{
    return get();
}

template <typename T>
inline auto asset<T>::operator*() const -> type&
{
    return *get();
}

template <typename T>
inline auto asset<T>::get() const -> type*
{
    return !_object.expired() ? _object.lock().get() : nullptr;
}

template <typename T>
inline void asset<T>::unload(bool greedy)
{
    if (_loader) {
        _loader->unload(*this, greedy);
    }

    assert(!is_ready());
    set_status(status::Unloaded);
}

template <typename T>
inline auto asset<T>::reload() -> bool
{
    return _loader ? _loader->reload(*this) : false;
}

template <typename T>
inline auto asset<T>::get_name() const -> string const&
{
    return _name;
}

template <typename T>
inline auto asset<T>::get_status() const -> status
{
    return _status;
}

template <typename T>
inline auto asset<T>::is_expired() const -> bool
{
    return _object.expired();
}

template <typename T>
inline asset<T>::operator bool() const
{
    return !is_expired();
}

template <typename T>
inline auto asset<T>::is_ready() const -> bool
{
    return _status == status::Loaded && !is_expired();
}

template <typename T>
inline void asset<T>::set_status(status status)
{
    _status = status;
}

template <typename T>
inline auto asset<T>::reset(std::weak_ptr<T> ptr)
{
    _object = ptr;
}

////////////////////////////////////////////////////////////

template <typename T>
inline asset_ptr<T>::asset_ptr(std::nullptr_t)
    : _asset {nullptr}
{
}

template <typename T>
inline asset_ptr<T>::asset_ptr(std::shared_ptr<asset<T>> ptr)
    : _asset {std::move(ptr)}
{
}

template <typename T>
template <BaseOfOrDerivedFrom<T> U>
inline asset_ptr<T>::asset_ptr(asset_ptr<U> const& other) noexcept
{
    *this = other;
}

template <typename T>
template <BaseOfOrDerivedFrom<T> U>
inline auto asset_ptr<T>::operator=(asset_ptr<U> const& other) noexcept -> asset_ptr<T>&
{
    _asset = std::make_shared<asset<T>>(asset<T> {*other._asset.get()});
    return *this;
}

template <typename T>
inline auto asset_ptr<T>::operator->() const -> type*
{
    return get_obj();
}

template <typename T>
inline auto asset_ptr<T>::operator*() const -> type&
{
    return *get_obj();
}

template <typename T>
inline auto asset_ptr<T>::get() const -> asset<type>*
{
    return _asset.get();
}

template <typename T>
inline auto asset_ptr<T>::get_obj() const -> type*
{
    return _asset->get();
}

template <typename T>
inline asset_ptr<T>::operator bool() const
{
    return !is_expired();
}

template <typename T>
inline auto asset_ptr<T>::is_expired() const -> bool
{
    return !_asset || _asset->is_expired();
}

template <typename T>
inline auto asset_ptr<T>::is_ready() const -> bool
{
    return _asset && _asset->is_ready();
}

template <typename T>
inline void asset_ptr<T>::reset()
{
    if (_asset) {
        _asset.reset();
    }
}

template <typename T>
inline auto asset_ptr<T>::get_use_count() const -> i32
{
    return _asset.use_count();
}

template <typename T>
inline auto operator==(asset_ptr<T> const& left, asset_ptr<T> const& right) -> bool
{
    if ((!left.is_expired() && right.is_expired()) || (left.is_expired() && !right.is_expired())) {
        return false;
    }

    if (left.is_expired() && right.is_expired()) {
        return true;
    }

    return left.get_obj() == right.get_obj();
}

////////////////////////////////////////////////////////////

template <typename T>
inline manual_asset_ptr<T>::manual_asset_ptr(string const& name, auto&&... args)
    : _object {std::make_shared<T>(args...)}
    , _assetPtr {std::make_shared<asset<T>>(name, _object, nullptr)}
{
}

template <typename T>
inline auto manual_asset_ptr<T>::operator->() const -> type*
{
    return _object.get();
}

template <typename T>
inline auto manual_asset_ptr<T>::operator*() const -> type&
{
    return *_object.get();
}

template <typename T>
template <BaseOfOrDerivedFrom<T> U>
inline manual_asset_ptr<T>::operator asset_ptr<U>() const
{
    return _assetPtr;
}

template <typename T>
inline auto manual_asset_ptr<T>::get_obj() const -> type*
{
    return _object.get();
}

template <typename T>
inline void manual_asset_ptr<T>::reset()
{
    _object.reset();
    _assetPtr.reset();
}

}
