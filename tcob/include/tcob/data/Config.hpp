// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/TypeFactory.hpp"

namespace tcob::data {
////////////////////////////////////////////////////////////

class entry;
class array;
class object;

using cfg_array_entries  = std::vector<entry>;
using cfg_object_entries = std::vector<std::pair<string, entry>>;
using cfg_value          = std::variant<std::monostate, i64, f64, bool, utf8_string, array, object>;

////////////////////////////////////////////////////////////

template <typename T>
struct converter;

template <typename T>
concept ConvertibleTo =
    requires(cfg_value& config, T const& t) {
        { converter<std::remove_cvref_t<T>>::To(config, t) };
    };

template <typename T>
concept ConvertibleFrom =
    requires(cfg_value& config, T& t) {
        { converter<T>::From(config, t) };
        { converter<T>::IsType(config) };
    };

////////////////////////////////////////////////////////////

enum class type : u8 {
    Null,
    String,
    Float,
    Integer,
    Bool,
    Array,
    Object
};

////////////////////////////////////////////////////////////

enum class error_code : u8 {
    Ok,
    Undefined,
    TypeMismatch
};

////////////////////////////////////////////////////////////

class TCOB_API text_reader : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<text_reader>> {
        static inline char const* ServiceName {"data::text_reader::factory"};
    };

    virtual ~text_reader() = default;

    auto virtual read_as_object(utf8_string_view txt) -> std::optional<object> = 0;
    auto virtual read_as_array(utf8_string_view txt) -> std::optional<array>   = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API text_writer : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<text_writer>> {
        static inline char const* ServiceName {"data::text_writer::factory"};
    };

    virtual ~text_writer() = default;

    auto virtual write(io::ostream& stream, object const& obj) -> bool = 0;
    auto virtual write(io::ostream& stream, array const& arr) -> bool  = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API binary_reader : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<binary_reader>> {
        static inline char const* ServiceName {"data::binary_reader::factory"};
    };

    virtual ~binary_reader() = default;

    auto virtual read_as_object(io::istream& stream) -> std::optional<object> = 0;
    auto virtual read_as_array(io::istream& stream) -> std::optional<array>   = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API binary_writer : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<binary_writer>> {
        static inline char const* ServiceName {"data::binary_writer::factory"};
    };

    virtual ~binary_writer() = default;

    auto virtual write(io::ostream& stream, object const& obj) -> bool = 0;
    auto virtual write(io::ostream& stream, array const& arr) -> bool  = 0;
};

////////////////////////////////////////////////////////////

}
