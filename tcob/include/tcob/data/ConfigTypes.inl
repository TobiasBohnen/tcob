// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ConfigTypes.hpp"

#include <numeric>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::data::config {

////////////////////////////////////////////////////////////

template <typename Impl, typename Container>
inline base_type<Impl, Container>::base_type(std::shared_ptr<Container> const& entries) noexcept
    : _values {entries}
{
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::load(path const& file, bool skipBinary) noexcept -> load_status
{
    io::ifstream fs {file};
    return load(fs, io::get_extension(file), skipBinary);
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::load(io::istream& in, string const& ext, bool skipBinary) noexcept -> load_status
{
    if (!in) { return load_status::Error; }
    return on_load(in, ext, skipBinary);
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::load_async(path const& file, bool skipBinary) noexcept -> std::future<load_status>
{
    return std::async(std::launch::async, [&, file, skipBinary] { return load(file, skipBinary); });
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
    if (auto txtWriter {locate_service<text_writer::factory>().create(ext)}) {
        return txtWriter->write(out, *static_cast<Impl const*>(this));
    }

    if (auto binWriter {locate_service<binary_writer::factory>().create(ext)}) {
        return binWriter->write(out, *static_cast<Impl const*>(this));
    }

    return false;
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::begin() -> Container::iterator
{
    return _values->begin();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::begin() const -> Container::const_iterator
{
    return _values->begin();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::end() -> Container::iterator
{
    return _values->end();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::end() const -> Container::const_iterator
{
    return _values->end();
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
inline auto base_type<Impl, Container>::values() -> Container*
{
    return _values.get();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::values() const -> Container*
{
    return _values.get();
}

template <typename Impl, typename Container>
inline auto base_type<Impl, Container>::swap(Impl& other)
{
    return std::swap(_values, other._values);
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

template <ConvertibleFrom T, typename... Keys>
inline auto object::as(string_view key, Keys&&... keys) const -> T
{
    return get<T>(key, std::forward<Keys>(keys)...).value();
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
inline auto object::get(string_view key) const -> result<T>
{
    if (auto it {find(key)}; it != values()->end()) {
        return it->second.template get<T>();
    }

    // If the key was not found, return a default-constructed value and an error code
    return result<T> {error_code::Undefined};
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::get(string_view key, string_view subkey, Keys&&... keys) const -> result<T>
{
    if (auto it {find(key)}; it != values()->end()) {
        auto const& ent {it->second};
        if (object sub; ent.try_get(sub)) {                         // If the value is a object (a nested key-value pair)
            return sub.get<T>(subkey, std::forward<Keys>(keys)...); // Recursively search the nested object for the value
        }
    }

    // If the key was not found, return a default-constructed value and an error code
    return result<T> {error_code::Undefined};
}

template <ConvertibleFrom T>
inline auto object::get(string_view key, isize index) const -> result<T>
{
    if (auto it {find(key)}; it != values()->end()) {
        auto const& ent {it->second};
        if (array sub; ent.try_get(sub)) { // If the value is an array
            return sub.get<T>(index);      // Recursively search the array for the value
        }
    }

    // If the key was not found, return a default-constructed value and an error code
    return result<T> {error_code::Undefined};
}

template <ConvertibleFrom T>
inline auto object::try_get(T& value, string_view key) const -> bool
{
    if (auto it {find(key)}; it != values()->end()) {
        return it->second.try_get(value);
    }

    return false;
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::try_get(T& value, string_view key, string_view subkey, Keys&&... keys) const -> bool
{
    if (auto it {find(key)}; it != values()->end()) {
        auto const& ent {it->second};
        if (object sub; ent.try_get(sub)) {                                    // If the value is a object (a nested key-value pair)
            return sub.try_get<T>(value, subkey, std::forward<Keys>(keys)...); // Recursively search the nested object for the value
        }
    }

    // If the key was not found, return a default-constructed value and an error code
    return false;
}

template <ConvertibleTo Value>
inline void object::set(string_view key, Value&& value)
{
    if (auto it {find(key)}; it != values()->end()) {
        it->second.set(std::forward<Value>(value));
        return;
    }

    // key not found -> add new entry
    entry ent; // TODO: -> constructor
    ent.set(value);
    add_entry(key, ent);
}

template <typename... KeysOrValue>
inline void object::set(string_view key, string_view subkey, KeysOrValue&&... keys)
{
    if (auto it {find(key)}; it != values()->end()) {
        auto&  ent {it->second};
        object sub {};
        if (!ent.try_get(sub)) { ent.set(sub); }                    // Convert the value to a object
        return sub.set(subkey, std::forward<KeysOrValue>(keys)...); // Recursively set the value in the nested object
    }

    using last_type = typename std::remove_cvref_t<detail::last_element_t<KeysOrValue...>>;
    if constexpr (!std::is_same_v<last_type, std::nullptr_t>) {
        // key not found -> add new object
        add_entry(key, object {});
        set(key, subkey, std::forward<KeysOrValue>(keys)...);
    }
}

template <ConvertibleTo Value>
inline void object::set(string_view key, isize index, Value&& value)
{
    if (auto it {find(key)}; it != values()->end()) {
        auto& ent {it->second};
        array sub {};
        if (!ent.try_get(sub)) { ent.set(sub); }           // Convert the value to an array
        return sub.set(index, std::forward<Value>(value)); // Recursively set the value in the array
    }

    // key not found -> add new array
    add_entry(key, array {});
    set(key, index, value);
}

template <ConvertibleFrom T>
inline auto object::is(string_view key) const -> bool
{
    if (auto it {find(key)}; it != values()->end()) { return it->second.template is<T>(); }

    return false;
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::is(string_view key, string_view subkey, Keys&&... keys) const -> bool
{
    if (auto it {find(key)}; it != values()->end()) {
        if (object sub {}; it->second.try_get(sub)) {              // If the value is a object (a nested key-value pair)
            return sub.is<T>(subkey, std::forward<Keys>(keys)...); // Recursively search the nested object for the value
        }
    }

    return false;
}

template <ConvertibleFrom T>
inline auto object::is(string_view key, isize index) const -> bool
{
    if (auto it {find(key)}; it != values()->end()) {
        if (array sub; it->second.try_get(sub)) { // If the value is an array
            return sub.is<T>(index);              // Recursively search the array for the value
        }
    }

    return false;
}

inline auto object::has(string_view key, auto&&... keys) const -> bool
{
    if (auto it {find(key)}; it != values()->end()) {
        if constexpr (sizeof...(keys) > 0) {
            if (object sub {}; it->second.try_get(sub)) {
                return sub.has(keys...);
            }
        } else {
            return true;
        }
    }

    return false;
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
inline auto array::as(isize index) const -> T
{
    return get<T>(index).value();
}

template <typename T, typename... Args>
inline auto array::make(auto&&... indices) const -> T
{
    constexpr usize argsCount {sizeof...(Args)};
    constexpr usize indCount {sizeof...(indices)};

    std::array<isize, argsCount> inds;

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
inline auto array::get(isize index) const -> result<T>
{
    if (index < 0 || index >= size()) { return result<T> {error_code::Undefined}; }

    return (*values())[static_cast<usize>(index)].get<T>();
}

template <ConvertibleTo T>
inline void array::set(isize index, T&& value)
{
    if (index >= 0) {
        if (index >= size()) {
            values()->resize(static_cast<usize>(index + 1));
        }

        (*values())[static_cast<usize>(index)].set(std::forward<T>(value));
    }
}

template <ConvertibleFrom T>
inline auto array::is(isize index) const -> bool
{
    if (index < 0 || index >= size()) { return false; }

    return (*values())[static_cast<usize>(index)].is<T>();
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
inline auto entry::as() const -> T
{
    return get<T>().value();
}

template <typename T>
inline auto entry::get() const -> result<T>
{
    T                retValue {};
    error_code const result {converter<T>::From(_value, retValue) ? error_code::Ok : error_code::TypeMismatch};
    return make_result(std::move(retValue), result);
}

template <typename T>
inline auto entry::try_get(T& value) const -> bool
{
    return converter<T>::From(_value, value);
}

template <typename T>
inline void entry::set(T&& value)
{
    converter<std::remove_cvref_t<T>>::To(_value, std::forward<T>(value));
}

template <typename T>
inline void entry::set_value(T const& value)
{
    _value = value;
}

template <typename T>
inline auto entry::is() const -> bool
{
    return converter<std::remove_cvref_t<T>>::IsType(_value);
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

    return std::all_of(left.begin(), left.end(), [&right](auto const& entry) {
        return right.has(entry.first) && *right.get_entry(entry.first) == entry.second;
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
