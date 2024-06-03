// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ConfigTypes.hpp"

#include "tcob/data/ConfigConversions.hpp" // IWYU pragma: keep

namespace tcob::data::config {

////////////////////////////////////////////////////////////

template <ConvertibleFrom T, typename... Keys>
inline auto object::as(string const& key, Keys&&... keys) const -> T
{
    return get<T>(key, std::forward<Keys>(keys)...).value();
}

template <ConvertibleFrom T>
inline auto object::get(string const& key) const -> result<T>
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        return it->second.template get<T>();
    }

    // If the key was not found, return a default-constructed value and an error code
    return result<T> {error_code::Undefined};
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::get(string const& key, string const& subkey, Keys&&... keys) const -> result<T>
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        auto const& ent {it->second};
        if (object sub; ent.try_get(sub)) {                         // If the value is a object (a nested key-value pair)
            return sub.get<T>(subkey, std::forward<Keys>(keys)...); // Recursively search the nested object for the value
        }
    }

    // If the key was not found, return a default-constructed value and an error code
    return result<T> {error_code::Undefined};
}

template <ConvertibleFrom T>
inline auto object::get(string const& key, isize index) const -> result<T>
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        auto const& ent {it->second};
        if (array sub; ent.try_get(sub)) { // If the value is an array
            return sub.get<T>(index);      // Recursively search the array for the value
        }
    }

    // If the key was not found, return a default-constructed value and an error code
    return result<T> {error_code::Undefined};
}

template <ConvertibleFrom T>
inline auto object::try_get(T& value, string const& key) const -> bool
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        return it->second.template try_get(value);
    }

    return false;
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::try_get(T& value, string const& key, string const& subkey, Keys&&... keys) const -> bool
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        auto const& ent {it->second};
        if (object sub; ent.try_get(sub)) {                                    // If the value is a object (a nested key-value pair)
            return sub.try_get<T>(value, subkey, std::forward<Keys>(keys)...); // Recursively search the nested object for the value
        }
    }

    // If the key was not found, return a default-constructed value and an error code
    return false;
}

template <ConvertibleTo Value>
inline void object::set(string const& key, Value&& value)
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        it->second.set(std::forward<Value>(value));
        return;
    }

    // key not found -> add new entry
    entry ent; // TODO: -> constructor
    ent.set(value);
    add_entry(key, ent);
}

template <typename... KeysOrValue>
inline void object::set(string const& key, string const& subkey, KeysOrValue&&... keys)
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
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
inline void object::set(string const& key, isize index, Value&& value)
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
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
inline auto object::is(string const& key) const -> bool
{
    auto it {find_key(key)};
    if (it != _kvps->end()) { return it->second.template is<T>(); }

    return false;
}

template <ConvertibleFrom T, typename... Keys>
inline auto object::is(string const& key, string const& subkey, Keys&&... keys) const -> bool
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        if (object sub {}; it->second.try_get(sub)) {              // If the value is a object (a nested key-value pair)
            return sub.is<T>(subkey, std::forward<Keys>(keys)...); // Recursively search the nested object for the value
        }
    }

    return false;
}

template <ConvertibleFrom T>
inline auto object::is(string const& key, isize index) const -> bool
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
        if (array sub; it->second.try_get(sub)) { // If the value is an array
            return sub.is<T>(index);              // Recursively search the array for the value
        }
    }

    return false;
}

inline auto object::has(string const& key, auto&&... keys) const -> bool
{
    auto it {find_key(key)};
    if (it != _kvps->end()) {
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
    ((add(values)), ...);
}

template <ConvertibleTo T>
inline array::array(std::span<T> value)
    : array {}
{
    _values->insert(_values->end(), value.begin(), value.end());
}

template <ConvertibleFrom T>
inline auto array::get(isize index) const -> result<T>
{
    if (index < 0 || index >= get_size()) { return result<T> {error_code::Undefined}; }

    return (*_values)[static_cast<usize>(index)].get<T>();
}

template <ConvertibleTo T>
inline void array::set(isize index, T&& value)
{
    if (index >= 0) {
        if (index >= get_size()) {
            _values->resize(static_cast<usize>(index + 1));
        }

        (*_values)[static_cast<usize>(index)].set(std::forward<T>(value));
    }
}

template <ConvertibleFrom T>
inline auto array::is(isize index) const -> bool
{
    if (index < 0 || index >= get_size()) { return false; }

    return (*_values)[static_cast<usize>(index)].is<T>();
}

template <ConvertibleTo T>
inline void array::add(T const& addValue)
{
    _values->emplace_back().set(addValue);
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
    if (left._kvps->size() != right._kvps->size()) {
        return false;
    }

    return std::all_of(left.begin(), left.end(), [&right](auto const& entry) {
        return right.has(entry.first) && *right.get_entry(entry.first) == entry.second;
    });
}

inline auto operator==(array const& left, array const& right) -> bool
{
    if (left._values->size() != right._values->size()) {
        return false;
    }

    for (isize i {0}; i < left.get_size(); ++i) {
        if (*left.get_entry(i) != *right.get_entry(i)) {
            return false;
        }
    }

    return true;
}

}
