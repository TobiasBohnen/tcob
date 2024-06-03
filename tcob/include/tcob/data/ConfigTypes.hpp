// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <future>
#include <memory>
#include <optional>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Proxy.hpp"
#include "tcob/data/Config.hpp"

namespace tcob::data::config {
////////////////////////////////////////////////////////////

struct comment {
    string Text;
};

////////////////////////////////////////////////////////////

class TCOB_API object {
    friend auto operator==(object const& left, object const& right) -> bool;

public:
    object() noexcept;
    object(std::shared_ptr<cfg_object_entries> const& entries) noexcept;

    auto operator[](string const& key) -> proxy<object, string>;
    auto operator[](string const& key) const -> proxy<object const, string> const;

    auto load(path const& file, bool skipBinary = false) noexcept -> load_status;
    auto load(istream& in, string const& ext, bool skipBinary = false) noexcept -> load_status;
    auto load_async(path const& file, bool skipBinary = false) noexcept -> std::future<load_status>;
    auto parse(string_view config, string const& ext) noexcept -> bool;

    auto save(path const& file) const -> bool;
    auto save(ostream& out, string const& ext) const -> bool;

    auto begin() -> cfg_object_entries::iterator;
    auto begin() const -> cfg_object_entries::const_iterator;
    auto end() -> cfg_object_entries::iterator;
    auto end() const -> cfg_object_entries::const_iterator;

    auto empty() const -> bool;
    auto get_size() const -> isize;

    void clear();

    template <ConvertibleFrom T, typename... Keys>
    auto as(string const& key, Keys&&... keys) const -> T;

    template <ConvertibleFrom T>
    auto get(string const& key) const -> result<T>;
    template <ConvertibleFrom T, typename... Keys>
    auto get(string const& key, string const& subkey, Keys&&... keys) const -> result<T>;
    template <ConvertibleFrom T>
    auto get(string const& key, isize index) const -> result<T>;

    template <ConvertibleFrom T>
    auto try_get(T& value, string const& key) const -> bool;
    template <ConvertibleFrom T, typename... Keys>
    auto try_get(T& value, string const& key, string const& subkey, Keys&&... keys) const -> bool;

    auto get_type(string const& key) const -> type;
    auto get_entry(string const& key) const -> entry*;

    template <ConvertibleTo Value>
    void set(string const& key, Value&& val);
    template <typename... KeysOrValue>
    void set(string const& key, string const& subkey, KeysOrValue&&... keys);
    template <ConvertibleTo Value>
    void set(string const& key, isize index, Value&& val);
    void set(string const& key, std::nullptr_t);

    void set_entry(string const& key, entry const& entry);

    template <ConvertibleFrom T>
    auto is(string const& key) const -> bool;
    template <ConvertibleFrom T, typename... Keys>
    auto is(string const& key, string const& subkey, Keys&&... keys) const -> bool;
    template <ConvertibleFrom T>
    auto is(string const& key, isize index) const -> bool;

    auto has(string const& key, auto&&... keys) const -> bool;

    auto clone(bool deep = false) const -> object;
    void merge(object const& other, bool onConflictTakeOther = true);

    auto str() const -> string;

    auto static Parse(string_view config, string const& ext) -> std::optional<object>; // TODO: change to result

private:
    void add_entry(string const& key, entry const& entry);

    auto find_key(string const& key) -> cfg_object_entries::iterator;
    auto find_key(string const& key) const -> cfg_object_entries::const_iterator;

    std::shared_ptr<cfg_object_entries> _kvps;
};

template <>
struct converter<object> {
    auto static IsType(cfg_value const& config) -> bool;
    auto static From(cfg_value const& config, object& value) -> bool;
    void static To(cfg_value& config, object const& value);
};

////////////////////////////////////////////////////////////

class TCOB_API array {
    friend auto operator==(array const& left, array const& right) -> bool;

public:
    array() noexcept;
    template <ConvertibleTo... Ts>
    explicit array(Ts... values);
    template <ConvertibleTo T>
    explicit array(std::span<T> value);

    auto operator[](isize index) -> proxy<array, isize>;
    auto operator[](isize index) const -> proxy<array const, isize>;
    auto load(path const& file, bool skipBinary = false) -> load_status;
    auto load(istream& in, string const& ext, bool skipBinary = false) -> load_status;
    auto load_async(path const& file) -> std::future<load_status>;
    auto parse(string_view config, string const& ext) -> bool;

    auto save(path const& file) const -> bool;
    auto save(ostream& out, string const& ext) const -> bool;

    auto begin() -> cfg_array_entries::iterator;
    auto begin() const -> cfg_array_entries::const_iterator;
    auto end() -> cfg_array_entries::iterator;
    auto end() const -> cfg_array_entries::const_iterator;

    auto empty() const -> bool;
    auto get_size() const -> isize;

    void clear();

    template <ConvertibleFrom T>
    auto get(isize index) const -> result<T>;

    auto get_type(isize index) const -> type;
    auto get_entry(isize index) const -> entry*;

    template <ConvertibleTo T>
    void set(isize index, T&& value);

    template <ConvertibleFrom T>
    auto is(isize index) const -> bool;

    template <ConvertibleTo T>
    void add(T const& addValue);

    void add_entry(entry const& newEntry);

    void pop_back();

    auto clone(bool deep = false) const -> array;

    auto str() const -> string;

    auto static Parse(string_view config, string const& ext) -> std::optional<array>; // TODO: change to result

private:
    std::shared_ptr<cfg_array_entries> _values;
};

template <>
struct converter<array> {
    auto static IsType(cfg_value const& config) -> bool;
    auto static From(cfg_value const& config, array& value) -> bool;
    void static To(cfg_value& config, array const& value);
};

////////////////////////////////////////////////////////////

class TCOB_API entry {
    friend auto operator==(entry const& left, entry const& right) -> bool;

public:
    entry() noexcept;
    entry(entry const& other)                    = default;
    auto operator=(entry const& other) -> entry& = default;
    template <typename T>
    entry(T val);

    template <typename T>
    auto as() const -> T;

    template <typename T>
    auto get() const -> result<T>;

    template <typename T>
    auto try_get(T& value) const -> bool;

    template <typename T>
    void set(T&& value);

    template <typename T>
    void set_value(T const& value);

    template <typename T>
    auto is() const -> bool;

    auto get_comment() const -> comment const&;
    void set_comment(comment const& comment);

private:
    cfg_value _value;
    comment   _comment;
};

////////////////////////////////////////////////////////////

}

namespace tcob::literals {
auto operator""_ini(char const* str, usize) -> tcob::data::config::object;
auto operator""_json(char const* str, usize) -> tcob::data::config::object;
auto operator""_xml(char const* str, usize) -> tcob::data::config::object;
auto operator""_yaml(char const* str, usize) -> tcob::data::config::object;
}

#include "ConfigTypes.inl"
