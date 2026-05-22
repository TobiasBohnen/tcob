// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ConfigTypes.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <future>
#include <memory>
#include <numeric>
#include <span>
#include <tuple>
#include <type_traits>
#include <variant>

#include "tcob/core/Common.hpp"
#include "tcob/core/Proxy.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/data/Config.hpp"

namespace tcob::data {

////////////////////////////////////////////////////////////

template <typename Impl, typename Container>
inline base_type<Impl, Container>::base_type(std::shared_ptr<Container> const& entries) noexcept
    : _values {entries}
{
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::load(path const& file, bool skipBinary) noexcept -> bool
{
    io::ifstream fs {file};
    return load(fs, io::get_extension(file), skipBinary);
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::load(io::istream& in, string const& ext, bool skipBinary) noexcept -> bool
{
    if (!in) { return false; }
    return on_load(in, ext, skipBinary);
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::load_async(path const& file, bool skipBinary) noexcept -> std::future<bool>
{
    return locate_service<task_manager>().run_async<bool>([&, file, skipBinary] mutable { return load(file, skipBinary); });
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::save(path const& file) const noexcept -> bool
{
    io::ofstream of {file};
    return save(of, io::get_extension(file));
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::save(io::ostream& out, string const& ext) const noexcept -> bool
{
    if (auto txtWriter {create_from_factory<text_writer>(ext)}) {
        return txtWriter->write(out, *static_cast<Impl const*>(this));
    }

    if (auto binWriter {create_from_factory<binary_writer>(ext)}) {
        return binWriter->write(out, *static_cast<Impl const*>(this));
    }

    return false;
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::begin(this auto&& self) -> decltype(auto)
{
    return self._values->begin();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::end(this auto&& self) -> decltype(auto)
{
    return self._values->end();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::empty() const -> bool
{
    return _values->empty();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::size() const -> isize
{
    return std::ssize(*_values);
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::capacity() const -> usize
{
    return _values->capacity();
}

template <typename Impl, typename Container>
inline void base_type<Impl, Container>::reserve(usize cap)
{
    _values->reserve(cap);
}

template <typename Impl, typename Container>
inline void base_type<Impl, Container>::clear()
{
    _values->clear();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::values(this auto&& self) -> decltype(auto)
{
    return self._values.get();
}

template <typename Impl, typename Container>
inline void base_type<Impl, Container>::swap(Impl& other)
{
    std::swap(_values, other._values);
}

template <typename Impl, typename Container>
template <typename Key>
inline auto base_type<Impl, Container>::get_type(Key key) const -> type
{
    auto* that {static_cast<Impl const*>(this)};
    if (that->template is<string>(key)) { return type::String; }
    if (that->template is<i64>(key)) { return type::Integer; }
    if (that->template is<f64>(key)) { return type::Float; }
    if (that->template is<bool>(key)) { return type::Bool; }
    if (that->template is<array>(key)) { return type::Array; }
    if (that->template is<object>(key)) { return type::Object; }

    return type::Null;
}

////////////////////////////////////////////////////////////

template <typename Key>
inline auto object::operator[](Key key) -> proxy<object, Key>
{
    return proxy<object, Key> {*this, std::tuple {key}};
}

template <typename Key>
inline auto object::operator[](Key key) const -> proxy<object const, Key> const
{
    return proxy<object const, Key> {*this, std::tuple {key}};
}

template <typename... Args, typename T>
inline auto object::try_make(T& value, auto&&... keys) const -> bool
{
    constexpr usize argsCount {sizeof...(Args)};
    constexpr usize keyCount {sizeof...(keys)};
    static_assert(argsCount == keyCount);

    std::tuple<Args...> tup {};
    if (std::apply(
            [&](auto&&... item) {
                return (try_get(item, keys) && ...);
            },
            tup)) {
        value = std::make_from_tuple<T>(tup);
        return true;
    }
    return false;
}

template <ConvertibleFrom T>
inline auto object::get() const -> std::expected<T, error_code>
{
    entry ent {*this};
    return ent.get<T>();
}

template <ConvertibleFrom T>
inline auto object::get(index_type key) const -> std::expected<T, error_code>
{
    auto const* val {get_entry(key)};
    if (!val) { return std::unexpected {error_code::Undefined}; }

    return val->template get<T>();
}

template <ConvertibleFrom T, typename Key, typename... Keys>
inline auto object::get(index_type key, Key const& subkey, Keys const&... keys) const -> std::expected<T, error_code>
{
    auto const* val {get_entry(key)};
    if (!val) { return std::unexpected {error_code::Undefined}; }

    if constexpr (std::is_convertible_v<Key, object::index_type>) {
        if (object sub; val->try_get(sub)) {
            return sub.get<T>(subkey, keys...);
        }
    }

    if constexpr (std::is_convertible_v<Key, array::index_type>) {
        if (array sub; val->try_get(sub)) {
            return sub.get<T>(subkey, keys...);
        }
    }

    return std::unexpected {error_code::TypeMismatch};
}

template <ConvertibleFrom T>
inline auto object::try_get(T& value, index_type key) const -> bool
{
    auto const* val {get_entry(key)};
    return val && val->try_get(value);
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::try_get(T& value, index_type key, index_type subkey, Keys const&... keys) const -> bool
{
    auto const* val {get_entry(key)};
    if (!val) { return false; }

    if (val) {
        if (object sub; val->try_get(sub)) {
            return sub.try_get<T>(value, subkey, keys...);
        }
    }

    return false;
}

template <typename Key, typename... KeysOrValue>
inline void object::set(index_type key, Key const& keyOrValue, KeysOrValue&&... keysOrValue)
{
    if constexpr (sizeof...(keysOrValue) == 0) {
        if (auto* val {get_entry(key)}) {
            val->set(keyOrValue);
            return;
        }

        // key not found -> add new entry
        entry ent; // TODO: -> constructor
        ent.set(keyOrValue);
        add_entry(key, ent);
    } else {
        auto* val {get_entry(key)};

        if constexpr (std::is_convertible_v<Key, object::index_type>) {
            if (val) {
                object sub {};
                if (!val->try_get(sub)) { val->set(sub); }
                sub.set(keyOrValue, std::forward<KeysOrValue>(keysOrValue)...);
                return;
            }

            using last_type = typename std::remove_cvref_t<detail::last_element_t<KeysOrValue...>>;
            if constexpr (!std::is_same_v<last_type, std::nullptr_t>) {
                // key not found -> add new object
                add_entry(key, entry {object {}});
                set(key, keyOrValue, std::forward<KeysOrValue>(keysOrValue)...);
                return;
            }
        }

        if constexpr (std::is_convertible_v<Key, array::index_type>) {
            if (val) {
                array sub {};
                if (!val->try_get(sub)) { val->set(sub); }
                sub.set(keyOrValue, std::forward<KeysOrValue>(keysOrValue)...);
                return;
            }

            // key not found -> add new array
            add_entry(key, entry {array {}});
            set(key, keyOrValue, std::forward<KeysOrValue>(keysOrValue)...);
            return;
        }
    }
}

template <ConvertibleFrom T>
inline auto object::is() const -> bool
{
    entry ent {*this};
    return ent.is<T>();
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::is(index_type key, Keys const&... keys) const
{
    auto const* val {get_entry(key)};
    if (!val) { return false; }

    if constexpr (sizeof...(keys) == 0) {
        return val->template is<T>();
    } else {
        if constexpr (std::is_convertible_v<detail::first_element_t<Keys...>, object::index_type>) {
            if (object sub {}; val->try_get(sub)) {
                return sub.is<T>(keys...);
            }
        }

        if constexpr (std::is_convertible_v<detail::first_element_t<Keys...>, array::index_type>) {
            if (array sub; val->try_get(sub)) {
                return sub.is<T>(keys...);
            }
        }

        return false;
    }
}

inline auto object::has(index_type key, auto&&... keys) const -> bool
{
    auto const* val {get_entry(key)};
    if (!val) { return false; }

    if constexpr (sizeof...(keys) > 0) {
        if (object sub {}; val->try_get(sub)) {
            return sub.has(keys...);
        }
    }

    return true;
}

////////////////////////////////////////////////////////////

template <ConvertibleTo... Ts>
inline array::array(Ts... values)
    : array {}
{
    reserve(sizeof...(values));
    ((add(values)), ...);
}

template <ConvertibleTo T>
inline array::array(std::span<T> value)
    : array {}
{
    values()->insert(end(), value.begin(), value.end());
}

template <ConvertibleFrom T>
inline auto array::as(index_type index) const -> T
{
    return get<T>(index).value();
}

template <typename T, typename... Args>
inline auto array::make(auto&&... indices) const -> T
{
    constexpr usize argsCount {sizeof...(Args)};
    constexpr usize indCount {sizeof...(indices)};

    std::array<index_type, argsCount> inds;

    if constexpr (indCount > 0) {
        static_assert(argsCount == indCount);
        inds = {indices...};
    } else {
        std::iota(inds.begin(), inds.end(), 0);
    }

    std::tuple<Args...> tup {};
    std::apply(
        [&](auto&&... item) {
            usize idx {0};
            ((item = get<Args>(inds[idx++]).value()), ...);
        },
        tup);

    return std::make_from_tuple<T>(tup);
}

template <ConvertibleFrom T>
inline auto array::get(index_type index) const -> std::expected<T, error_code>
{
    if (index < 0 || index >= size()) { return std::unexpected {error_code::Undefined}; }

    return get_entry(index)->get<T>();
}

template <ConvertibleFrom T, typename Key, typename... Keys>
inline auto array::get(index_type index, Key const& subkey, Keys const&... keys) const -> std::expected<T, error_code>
{
    auto const* val {get_entry(index)};
    if (!val) { return std::unexpected {error_code::Undefined}; }

    if constexpr (std::is_convertible_v<Key, array::index_type>) {
        if (array sub; val->try_get(sub)) {
            return sub.get<T>(subkey, keys...);
        }
    }

    if constexpr (std::is_convertible_v<Key, object::index_type>) {
        if (object sub; val->try_get(sub)) {
            return sub.get<T>(subkey, keys...);
        }
    }

    return std::unexpected {error_code::TypeMismatch};
}

template <typename Key, typename... KeysOrValue>
inline void array::set(index_type index, Key const& keyOrValue, KeysOrValue&&... keysOrValue)
{
    if (index >= 0) {
        if (index >= size()) {
            values()->resize(static_cast<usize>(index + 1));
        }
        auto* val {get_entry(index)};
        if (!val) { return; }

        if constexpr (sizeof...(keysOrValue) == 0) {
            get_entry(index)->set(keyOrValue);
        } else {
            if constexpr (std::is_convertible_v<Key, array::index_type>) {
                array sub {};
                if (!val->try_get(sub)) { val->set(sub); }
                sub.set(keyOrValue, std::forward<KeysOrValue>(keysOrValue)...);
                return;
            }

            if constexpr (std::is_convertible_v<Key, object::index_type>) {
                object sub {};
                if (!val->try_get(sub)) { val->set(sub); }
                sub.set(keyOrValue, std::forward<KeysOrValue>(keysOrValue)...);
                return;
            }
        }
    }
}

template <ConvertibleFrom T, typename... Keys>
inline auto array::is(index_type index, Keys const&... keys) const
{
    if (index < 0 || index >= size()) { return false; }

    auto const* val {get_entry(index)};
    if (!val) { return false; }

    if constexpr (sizeof...(keys) == 0) {
        return val->template is<T>();
    } else {
        if constexpr (std::is_convertible_v<detail::first_element_t<Keys...>, array::index_type>) {
            if (array sub; val->try_get(sub)) {
                return sub.is<T>(keys...);
            }
        }

        if constexpr (std::is_convertible_v<detail::first_element_t<Keys...>, object::index_type>) {
            if (object sub {}; val->try_get(sub)) {
                return sub.is<T>(keys...);
            }
        }
    }

    return false;
}

template <ConvertibleTo T>
inline void array::add(T const& addValue)
{
    values()->emplace_back().set(addValue);
}

////////////////////////////////////////////////////////////

template <typename T>
inline entry::entry(T val)
{
    set(std::move(val));
}

template <typename T>
inline auto entry::operator=(T const& other) -> entry&
{
    set(std::move(other));
    return *this;
}

template <typename T>
inline auto entry::as() const -> T
{
    return get<T>().value();
}

template <typename T>
inline auto entry::get() const -> std::expected<T, error_code>
{
    T retValue {};
    return try_get(retValue)
        ? std::expected<T, error_code> {std::move(retValue)}
        : std::unexpected {error_code::TypeMismatch};
}

template <typename T>
inline auto entry::try_get(T& value) const -> bool
{
    using type = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<type, object> || std::is_same_v<type, array>) {
        if (!std::holds_alternative<type>(_value)) { return false; }
        value = std::get<type>(_value);
        return true;
    } else {
        return base_converter<T>::From(_value, value);
    }
}

template <typename T>
inline void entry::set(T&& value)
{
    using type = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<type, object> || std::is_same_v<type, array>) {
        _value = value;
    } else {
        base_converter<type>::To(_value, std::forward<T>(value));
    }
}

template <typename T>
inline void entry::set_value(T const& value)
{
    _value = value;
}

template <typename T>
inline auto entry::is() const -> bool
{
    using type = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<type, object>) {
        return std::holds_alternative<object>(_value);
    } else if constexpr (std::is_same_v<type, array>) {
        return std::holds_alternative<array>(_value);
    } else {
        return base_converter<type>::IsType(_value);
    }
}

////////////////////////////////////////////////////////////

inline auto operator==(entry const& left, entry const& right) -> bool
{
    return left._value == right._value;
}

inline auto operator==(object const& left, object const& right) -> bool
{
    if (left.values()->size() != right.values()->size()) {
        return false;
    }

    return std::ranges::all_of(left, [&right](auto const& entry) {
        auto const* val {right.get_entry(entry.first)};
        return val && *val == entry.second;
    });
}

inline auto operator==(array const& left, array const& right) -> bool
{
    if (left.values()->size() != right.values()->size()) {
        return false;
    }

    for (isize i {0}; i < left.size(); ++i) {
        if (*left.get_entry(i) != *right.get_entry(i)) {
            return false;
        }
    }

    return true;
}
}
