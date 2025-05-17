// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cstddef>
#include <expected>
#include <future>
#include <initializer_list>
#include <memory>
#include <optional>
#include <span>
#include <utility>

#include "tcob/core/Common.hpp"
#include "tcob/core/Proxy.hpp"
#include "tcob/data/Config.hpp"

namespace tcob::data {
////////////////////////////////////////////////////////////

struct comment {
    string Text;
};

////////////////////////////////////////////////////////////

template <typename Impl, typename Container>
class base_type {
    using iterator       = Container::iterator;
    using const_iterator = Container::const_iterator;

public:
    base_type() noexcept = default;
    base_type(std::shared_ptr<Container> const& entries) noexcept;
    base_type(base_type const& other) noexcept                    = default;
    auto operator=(base_type const& other) noexcept -> base_type& = default;
    base_type(base_type&& other) noexcept                         = default;
    auto operator=(base_type&& other) noexcept -> base_type&      = default;
    virtual ~base_type()                                          = default;

    auto load(path const& file, bool skipBinary = false) noexcept -> load_status;
    auto load(io::istream& in, string const& ext, bool skipBinary = false) noexcept -> load_status;
    auto load_async(path const& file, bool skipBinary = false) noexcept -> std::future<load_status>;

    auto save(path const& file) const noexcept -> bool;
    auto save(io::ostream& out, string const& ext) const noexcept -> bool;

    auto begin(this auto&& self);
    auto end(this auto&& self);

    auto empty() const -> bool;
    auto size() const -> isize;
    auto capacity() const -> usize;
    void reserve(usize cap);
    void clear();

    template <typename Key>
    auto get_type(Key key) const -> type;

protected:
    auto virtual on_load(io::istream& in, string const& ext, bool skipBinary = false) noexcept -> load_status = 0;

    auto values(this auto&& self);

    void swap(Impl& other);

private:
    std::shared_ptr<Container> _values;
};

////////////////////////////////////////////////////////////

class TCOB_API object : public base_type<object, cfg_object_entries> {
    friend auto operator==(object const& left, object const& right) -> bool;

public:
    object() noexcept;
    object(std::initializer_list<std::pair<utf8_string, cfg_value>> items);
    object(std::shared_ptr<cfg_object_entries> const& entries) noexcept;

    auto operator[](string const& key) -> proxy<object, string>;
    auto operator[](string const& key) const -> proxy<object const, string> const;

    auto parse(string_view config, string const& ext) noexcept -> bool;

    template <ConvertibleFrom T, typename... Keys>
    auto as(string_view key, Keys&&... keys) const -> T;

    template <typename... Args, typename T>
    auto try_make(T& value, auto&&... keys) const -> bool;

    template <ConvertibleFrom T>
    auto get(string_view key) const -> std::expected<T, error_code>;
    template <ConvertibleFrom T, typename... Keys>
    auto get(string_view key, string_view subkey, Keys&&... keys) const -> std::expected<T, error_code>;
    template <ConvertibleFrom T>
    auto get(string_view key, isize index) const -> std::expected<T, error_code>;

    template <ConvertibleFrom T>
    auto try_get(T& value, string_view key) const -> bool;
    template <ConvertibleFrom T, typename... Keys>
    auto try_get(T& value, string_view key, string_view subkey, Keys&&... keys) const -> bool;

    template <ConvertibleTo Value>
    void set(string_view key, Value&& val);
    template <typename... KeysOrValue>
    void set(string_view key, string_view subkey, KeysOrValue&&... keys);
    template <ConvertibleTo Value>
    void set(string_view key, isize index, Value&& val);
    void set(string_view key, std::nullptr_t);

    template <ConvertibleFrom T>
    auto is(string_view key) const -> bool;
    template <ConvertibleFrom T, typename... Keys>
    auto is(string_view key, string_view subkey, Keys&&... keys) const -> bool;
    template <ConvertibleFrom T>
    auto is(string_view key, isize index) const -> bool;

    auto has(string_view key, auto&&... keys) const -> bool;

    auto clone(bool deep = false) const -> object;
    void merge(object const& other, bool onConflictTakeOther = true);

    auto str() const -> string;

    auto static Parse(string_view config, string const& ext) -> std::optional<object>; // TODO: change to result

    auto get_entry(string_view key) -> entry*;
    auto get_entry(string_view key) const -> entry const*;
    void set_entry(string_view key, entry const& entry);

protected:
    auto on_load(io::istream& in, string const& ext, bool skipBinary = false) noexcept -> load_status override;

private:
    void add_entry(string_view key, entry const& entry);
    void remove_entry(string_view key);

    auto find(string_view key) -> cfg_object_entries::iterator;
    auto find(string_view key) const -> cfg_object_entries::const_iterator;
};

template <>
struct converter<object> {
    auto static IsType(cfg_value const& config) -> bool;
    auto static From(cfg_value const& config, object& value) -> bool;
    void static To(cfg_value& config, object const& value);
};

////////////////////////////////////////////////////////////

class TCOB_API array : public base_type<array, cfg_array_entries> {
    friend auto operator==(array const& left, array const& right) -> bool;

public:
    array() noexcept;
    template <ConvertibleTo... Ts>
    explicit array(Ts... values);
    template <ConvertibleTo T>
    explicit array(std::span<T> value);

    auto operator[](isize index) -> proxy<array, isize>;
    auto operator[](isize index) const -> proxy<array const, isize>;

    auto parse(string_view config, string const& ext) -> bool;

    template <ConvertibleFrom T>
    auto as(isize index) const -> T;

    template <typename T, typename... Args>
    auto make(auto&&... indices) const -> T;

    template <ConvertibleFrom T>
    auto get(isize index) const -> std::expected<T, error_code>;

    template <ConvertibleTo T>
    void set(isize index, T&& value);

    template <ConvertibleFrom T>
    auto is(isize index) const -> bool;

    template <ConvertibleTo T>
    void add(T const& addValue);

    void pop_back();

    auto clone(bool deep = false) const -> array;

    auto str() const -> string;

    auto static Parse(string_view config, string const& ext) -> std::optional<array>; // TODO: change to result

    auto get_entry(isize index) const -> entry*;
    void add_entry(entry const& newEntry);

protected:
    auto on_load(io::istream& in, string const& ext, bool skipBinary = false) noexcept -> load_status override;
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
    auto get() const -> std::expected<T, error_code>;

    template <typename T>
    auto try_get(T& value) const -> bool;

    template <typename T>
    void set(T&& value);

    template <typename T>
    void set_value(T const& value);

    template <typename T>
    auto is() const -> bool;

    auto get_comment() const -> comment const&; // TODO: set_get_
    void set_comment(comment const& comment);

private:
    cfg_value _value;
    comment   _comment;
};

////////////////////////////////////////////////////////////

}

namespace tcob::literals {
auto operator""_ini(char const* str, usize) -> tcob::data::object;
auto operator""_json(char const* str, usize) -> tcob::data::object;
auto operator""_xml(char const* str, usize) -> tcob::data::object;
auto operator""_yaml(char const* str, usize) -> tcob::data::object;
}

#include "ConfigTypes.inl"
