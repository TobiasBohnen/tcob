// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <type_traits>

#include "tcob/core/Concepts.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"

namespace tcob {

namespace detail {
    ////////////////////////////////////////////////////////////

    template <typename T>
    class field_source final {
    public:
        using type              = std::remove_const_t<T>;
        using return_type       = type&;
        using const_return_type = type const&;

        field_source() = default;
        field_source(type value);

        auto get() -> return_type;
        auto get() const -> const_return_type;
        auto set(type const& value, bool force) -> bool;

    private:
        type _value {};
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class validating_field_source final {
    public:
        using type              = std::remove_const_t<T>;
        using return_type       = type&;
        using const_return_type = type const&;

        using validate_func = std::function<type(type const&)>;

        validating_field_source(validate_func val);
        validating_field_source(type value, validate_func val);

        auto get() -> return_type;
        auto get() const -> const_return_type;
        auto set(type const& value, bool force) -> bool;

    private:
        validate_func _validate;
        type          _value;
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class func_source final {
    public:
        using type              = T;
        using return_type       = type;
        using const_return_type = type const;

        using getter_func = std::function<type()>;
        using setter_func = std::function<void(type const&)>;

        func_source(getter_func get, setter_func set);
        func_source(type value, getter_func get, setter_func set);

        auto get() -> return_type;
        auto get() const -> const_return_type;
        auto set(type const& value, bool force) -> bool;

    private:
        getter_func _getter;
        setter_func _setter;
    };

}

// FIXME: https://developercommunity.visualstudio.com/t/Out-of-line-definition-of-a-constrained-/1306502
////////////////////////////////////////////////////////////

template <typename T, typename Source>
class prop_base final : public non_copyable {
public:
    using return_type       = typename Source::return_type;
    using const_return_type = typename Source::const_return_type;

    prop_base() = default;
    explicit prop_base(Source source)
        : _source {std::move(source)}
    {
    }
    explicit prop_base(T val)
        : _source {val}
    {
    }

    operator T() const
    {
        return _source.get();
    }

    auto operator=(T const& value) -> prop_base&
    {
        set(value, false);
        return *this;
    }

    void operator()(T const& value)
    {
        set(value, true);
    }

    auto operator->() const
    {
        if constexpr (OverloadsArrowOp<T>) {
            return _source.get();
        } else {
            static_assert(std::is_reference_v<return_type>);
            return &_source.get();
        }
    }

    auto operator*() -> return_type
    {
        return _source.get();
    }
    auto operator*() const -> const_return_type
    {
        return _source.get();
    }

    auto operator()() const -> const_return_type
    {
        return _source.get();
    }

    signal<T const> Changed;

private:
    void set(T const& value, bool force)
    {
        if (_source.set(value, force) && Changed.slot_count() > 0) {
            Changed(_source.get());
        }
    }

    Source _source;
};

template <Container T, typename Source>
class prop_base<T, Source> final : public non_copyable {
public:
    using value_type        = typename T::value_type;
    using return_type       = typename Source::return_type;
    using const_return_type = typename Source::const_return_type;

    prop_base() = default;
    explicit prop_base(Source source)
        : _source {std::move(source)}
    {
    }
    explicit prop_base(T val)
        : _source {val}
    {
    }

    operator T() const
    {
        return _source.get();
    }

    auto operator=(T const& value) -> prop_base&
    {
        set(value, false);
        return *this;
    }

    void operator()(T const& value)
    {
        set(value, true);
    }

    auto operator->() const
    {
        if constexpr (OverloadsArrowOp<T>) {
            return _source.get();
        } else {
            static_assert(std::is_reference_v<return_type>);
            return &_source.get();
        }
    }

    auto operator*() -> return_type
    {
        return _source.get();
    }
    auto operator*() const -> const_return_type
    {
        return _source.get();
    }

    auto operator()() const -> const_return_type
    {
        return _source.get();
    }

    signal<T const&> Changed;

    void add(value_type const& element)
    {
        if constexpr (std::is_reference_v<return_type>) {
            auto& vec {_source.get()};
            vec.push_back(element);
            Changed(vec);
        } else {
            auto vec {_source.get()};
            vec.push_back(element);
            set(vec, true);
        }
    }
    void set(usize index, value_type const& newValue)
    {
        if constexpr (std::is_reference_v<return_type>) {
            auto& vec {_source.get()};
            if (index < vec.size()) {
                vec[index] = newValue;
                Changed(vec);
            }
        } else {
            auto vec {_source.get()};
            if (index < vec.size()) {
                vec[index] = newValue;
                set(vec, true);
            }
        }
    }

private:
    void set(T const& value, bool force)
    {
        if (_source.set(value, force) && Changed.slot_count() > 0) {
            Changed(_source.get());
        }
    }

    Source _source;
};

////////////////////////////////////////////////////////////

template <typename T, typename Source>
auto constexpr operator+=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

template <typename T, typename Source>
auto constexpr operator+=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&;

template <typename T, typename Source>
auto constexpr operator-(prop_base<T, Source>& right) -> T;

template <typename T, typename Source>
auto constexpr operator-=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

template <typename T, typename Source>
auto constexpr operator-=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&;

template <typename T, typename Source>
auto constexpr operator/=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

template <typename T, typename Source>
auto constexpr operator/=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&;

template <typename T, typename Source>
auto constexpr operator*=(prop_base<T, Source>& left, T const& right) -> prop_base<T, Source>&;

template <typename T, typename Source>
auto constexpr operator*=(prop_base<T const, Source>& left, T const& right) -> prop_base<T const, Source>&;

template <typename T, typename Source>
auto constexpr operator==(prop_base<T, Source> const& left, T const& right) -> bool;

template <typename T, typename Source>
auto constexpr operator==(prop_base<T const, Source> const& left, T const& right) -> bool;

template <typename T, typename Source>
auto constexpr operator==(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> bool;

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T, Source> const& left, T const& right) -> std::partial_ordering;

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T const, Source> const& left, T const& right) -> std::partial_ordering;

template <typename T, typename Source>
auto constexpr operator<=>(prop_base<T, Source> const& left, prop_base<T, Source> const& right) -> std::partial_ordering;

////////////////////////////////////////////////////////////

// field-backed property.
template <typename T>
using prop = prop_base<T, detail::field_source<T>>;

// validating field-backed property.
template <typename T>
using prop_val = prop_base<T, detail::validating_field_source<T>>;

// function-backed property.
template <typename T>
using prop_fn = prop_base<T, detail::func_source<T>>;

}

#include "Property.inl"
