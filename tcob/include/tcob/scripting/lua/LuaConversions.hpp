// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// IWYU pragma: always_keep

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <filesystem>
    #include <functional>
    #include <optional>
    #include <span>
    #include <tuple>
    #include <type_traits>
    #include <unordered_set>
    #include <utility>
    #include <variant>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Concepts.hpp"
    #include "tcob/scripting/lua/Lua.hpp"
    #include "tcob/scripting/lua/LuaClosure.hpp"
    #include "tcob/scripting/lua/LuaTypes.hpp"

    #include "tcob/core/ext/magic_enum_reduced.hpp"

namespace tcob::scripting::lua {

template <typename T>
consteval auto get_stacksize() -> i32
{
    if constexpr (requires {{ converter<T>::StackSize()}; }) {
        return converter<T>::StackSize();
    } else {
        return 1;
    }
}

////functions/////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename R, typename... P>
struct converter<R(P...)> {
    void static To(state_view view, R (*value)(P...))
    {
        view.push_lightuserdata(reinterpret_cast<void*>(value));
        view.push_cclosure(
            [](lua_State* l) -> i32 {
                state_view                      s {l};
                detail::native_closure<R(P...)> cl {reinterpret_cast<R (*)(P...)>(s.to_userdata(state_view::GetUpvalueIndex(1)))};
                return cl(s);
            },
            1);
    }
};

template <typename R, typename... P>
struct converter<R (*)(P...)> {
    void static To(state_view view, R (*value)(P...))
    {
        view.push_lightuserdata(reinterpret_cast<void*>(value));
        view.push_cclosure(
            [](lua_State* l) -> i32 {
                state_view                      s {l};
                detail::native_closure<R(P...)> cl {reinterpret_cast<R (*)(P...)>(s.to_userdata(state_view::GetUpvalueIndex(1)))};
                return cl(s);
            },
            1);
    }
};

template <typename R, typename... P>
struct converter<std::function<R(P...)>*> {
    void static To(state_view view, std::function<R(P...)>* value)
    {
        view.push_lightuserdata(reinterpret_cast<void*>(value));
        view.push_cclosure(
            [](lua_State* l) -> i32 {
                state_view                      s {l};
                detail::native_closure<R(P...)> cl {*reinterpret_cast<std::function<R(P...)>*>(s.to_userdata(state_view::GetUpvalueIndex(1)))};
                return cl(s);
            },
            1);
    }
};

template <>
struct converter<detail::native_closure_base*> {
    void static To(state_view view, detail::native_closure_base* value)
    {
        view.push_lightuserdata(reinterpret_cast<void*>(value));
        view.push_cclosure(
            [](lua_State* l) -> i32 {
                state_view s {l};
                auto*      p {reinterpret_cast<detail::native_closure_base*>(s.to_userdata(state_view::GetUpvalueIndex(1)))};
                return (*p)(s);
            },
            1);
    }
};

////STL///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct converter<std::optional<T>> {
    static consteval auto StackSize() -> i32
    {
        return get_stacksize<T>();
    }

    auto static IsType(state_view, i32) -> bool
    {
        return true;
    }

    auto static From(state_view view, i32& idx, std::optional<T>& value) -> bool
    {
        if (idx > view.get_top() || !converter<T>::IsType(view, idx)) {
            value = std::nullopt;
        } else {
            T val {};
            converter<T>::From(view, idx, val);
            value = val;
        }

        return true;
    }

    void static To(state_view view, std::optional<T> const& value)
    {
        if (value) {
            converter<T>::To(view, *value);
        } else {
            view.push_nil();
        }
    }
};

template <>
struct converter<std::nullopt_t> {
    void static To(state_view view, std::nullopt_t const&)
    {
        view.push_nil();
    }
};

template <typename... P>
struct converter<std::variant<P...>> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return check_variant<P...>(view, idx);
    }

    auto static From(state_view view, i32& idx, std::variant<P...>& value) -> bool
    {
        return convert_from<P...>(view, idx, value);
    }

    void static To(state_view view, std::variant<P...> const& value)
    {
        std::visit([&view](auto&& item) { convert_to(view, item); }, value);
    }

private:
    template <typename T, typename... Ts>
    auto static convert_from(state_view view, i32& idx, std::variant<P...>& value) -> bool
    {
        if (converter<T>::IsType(view, idx)) {
            T val {};
            converter<T>::From(view, idx, val);
            value = val;
            return true;
        }

        if constexpr (sizeof...(Ts) > 0) {
            return convert_from<Ts...>(view, idx, value);
        } else {
            return false;
        }
    }

    template <typename R>
    void static convert_to(state_view view, R const& value)
    {
        converter<R>::To(view, value);
    }

    template <typename T, typename... Ts>
    auto static check_variant(state_view view, i32 idx) -> bool
    {
        if constexpr (sizeof...(Ts) > 0) {
            return converter<T>::IsType(view, idx) || check_variant<Ts...>(view, idx);
        } else {
            return converter<T>::IsType(view, idx);
        }
    }
};

template <Map T>
struct converter<T> {
    using key_type    = typename T::key_type;
    using mapped_type = typename T::mapped_type;

    auto static IsType(state_view view, i32 idx) -> bool
    {
        return check_map(view, idx);
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        if (view.is_table(idx)) {
            bool retValue {true};

            view.push_value(idx);               // stack: -1 => table
            view.push_nil();                    // stack: -1 => nil; -2 => table

            while (view.next(-2) && retValue) { // stack: -1 => value; -2 => key; -3 => table
                view.push_value(-2);            // stack: -1 => key; -2 => value; -3 => key; -4 => table
                key_type key {};
                i32      idx0 {-1}, idx1 {-2};
                retValue = converter<key_type>::From(view, idx0, key)
                    && converter<mapped_type>::From(view, idx1, value[key]);
                view.pop(2); // stack: -1 => key; -2 => table
            }

            view.pop(1);     // stack: -1 => table

            idx++;
            return retValue;
        }
        return false;
    }

    void static To(state_view view, T const& value)
    {
        view.create_table(0, static_cast<i32>(value.size()));
        for (auto& [key, val] : value) {
            converter<key_type>::To(view, key);
            converter<mapped_type>::To(view, val);
            view.set_table(-3);
        }
    }

private:
    auto static check_map(state_view view, i32 idx) -> bool
    {
        bool retValue {view.is_table(idx)};
        if (retValue) {
            view.push_value(idx);               // stack: -1 => table
            view.push_nil();                    // stack: -1 => nil; -2 => table

            while (view.next(-2) && retValue) { // stack: -1 => value; -2 => key; -3 => table
                view.push_value(-2);            // stack: -1 => key; -2 => value; -3 => key; -4 => table
                retValue = converter<key_type>::IsType(view, -1)
                    && converter<mapped_type>::IsType(view, -2);
                view.pop(2);                    // stack: -1 => key; -2 => table
            }

            view.pop(1);                        // stack: -1 => table
        }

        return retValue;
    }
};

template <Set T>
struct converter<T> {
    using key_type = typename T::key_type;

    auto static IsType(state_view view, i32 idx) -> bool
    {
        return check_set(view, idx);
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        bool retValue {view.is_table(idx)};
        if (retValue) {
            i64 const len {static_cast<i64>(view.raw_len(idx))};
            for (i64 i {1}; i <= len && retValue; ++i) {
                view.raw_get(idx, i);
                key_type val {};
                i32      idx2 {-1};
                retValue = converter<key_type>::From(view, idx2, val);
                view.pop(1);
                if (retValue) {
                    value.insert(val);
                }
            }
            idx++;
        }
        return retValue;
    }

    void static To(state_view view, T const& value)
    {
        view.create_table(0, static_cast<i32>(value.size()));

        for (i32 i {0}; auto& val : value) {
            converter<key_type>::To(view, val);
            view.raw_set(-2, ++i);
        }
    }

private:
    auto static check_set(state_view view, i32 idx) -> bool
    {
        bool retValue {view.is_table(idx)};
        if (retValue) {
            u64 const len {view.raw_len(idx)};

            for (u64 i {1}; i <= len && retValue; ++i) {
                view.raw_get(idx, static_cast<i64>(i));
                retValue = converter<key_type>::IsType(view, -1);
                view.pop(1);
            }
        }
        return retValue;
    }
};

template <typename... T>
struct converter<std::tuple<T...>> {
    static consteval auto StackSize() -> i32
    {
        return static_cast<i32>(std::tuple_size_v<std::tuple<T...>>);
    }

    auto static IsType(state_view view, i32 idx) -> bool
    {
        return (converter<T>::IsType(view, idx++) && ...);
    }

    auto static From(state_view view, i32& idx, std::tuple<T...>& value) -> bool
    {
        return std::apply(
            [view, &idx](auto&&... item) {
                return ((convert_from(view, idx, item)) && ...);
            },
            value);
    }

    void static To(state_view view, std::tuple<T...> const& value)
    {
        std::apply(
            [view](auto&&... item) {
                (convert_to(view, item), ...);
            },
            value);
    }

private:
    template <typename R>
    auto static convert_from(state_view view, i32& idx, R& value) -> bool
    {
        return converter<R>::From(view, idx, value);
    }

    template <typename R>
    void static convert_to(state_view view, R const& value)
    {
        converter<R>::To(view, value);
    }
};

template <typename K, typename V>
struct converter<std::pair<K, V>> {
    static consteval auto StackSize() -> i32
    {
        return 2;
    }

    auto static IsType(state_view view, i32 idx) -> bool
    {
        return converter<K>::IsType(view, idx) && converter<V>::IsType(view, idx + 1);
    }

    auto static From(state_view view, i32& idx, std::pair<K, V>& value) -> bool
    {
        K first {};
        if (converter<K>::From(view, idx, first)) {
            V second {};
            if (converter<V>::From(view, idx, second)) {
                std::pair pair {first, second};
                value.swap(pair);
                return true;
            }
        }
        return false;
    }

    void static To(state_view view, std::pair<K, V> const& value)
    {
        converter<K>::To(view, value.first);
        converter<V>::To(view, value.second);
    }
};

template <typename T>
struct converter<parameter_pack<T>> {

    void static To(state_view view, parameter_pack<T> const& value)
    {
        for (auto const& item : value.Items) {
            converter<T>::To(view, item);
        }
    }
};

template <typename T, usize Size>
struct converter<std::array<T, Size>> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return check_array(view, idx);
    }

    auto static From(state_view view, i32& idx, std::array<T, Size>& value) -> bool
    {
        bool retValue {view.is_table(idx) && view.raw_len(idx) == Size};
        for (usize i {1}; i <= Size && retValue; ++i) {
            view.raw_get(idx, static_cast<i64>(i));
            i32 idx0 {-1};
            retValue = converter<T>::From(view, idx0, value[i - 1]);
            view.pop(1);
        }

        idx++;

        return retValue;
    }

    void static To(state_view view, std::array<T, Size> const& value)
    {
        view.create_table(0, static_cast<i32>(Size));

        for (usize i {0}; i < Size; ++i) {
            converter<T>::To(view, value[i]);
            view.raw_set(-2, static_cast<i64>(i + 1));
        }
    }

private:
    auto static check_array(state_view view, i32 idx) -> bool
    {
        bool retValue {view.is_table(idx) && view.raw_len(idx) == Size};
        for (u64 i {1}; i <= Size && retValue; ++i) {
            view.raw_get(idx, static_cast<i64>(i));
            retValue = converter<T>::IsType(view, -1);
            view.pop(1);
        }
        return retValue;
    }
};

template <Container T>
struct converter<T> {
    using value_type = typename T::value_type;

    auto static IsType(state_view view, i32 idx) -> bool
    {
        return check_vector(view, idx);
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        bool retValue {view.is_table(idx)};
        if (retValue) {
            u64 const len {view.raw_len(idx)};
            value.resize(static_cast<usize>(len));

            for (usize i {1}; i <= len; ++i) {
                view.raw_get(idx, static_cast<i64>(i));
                value_type val {};
                i32        idx2 {-1};
                retValue = converter<value_type>::From(view, idx2, val);
                view.pop(1);
                if (retValue) {
                    value[i - 1] = val;
                } else {
                    value.resize(i - 1);
                    break;
                }
            }
            idx++;
        }
        return retValue;
    }

    void static To(state_view view, T const& value)
    {
        view.create_table(0, static_cast<i32>(value.size()));

        for (usize i {0}; i < value.size(); ++i) {
            converter<value_type>::To(view, value[i]);
            view.raw_set(-2, static_cast<i64>(i + 1));
        }
    }

private:
    auto static check_vector(state_view view, i32 idx) -> bool
    {
        bool retValue {view.is_table(idx)};
        if (retValue) {
            u64 const len {view.raw_len(idx)};
            if (len == 0) {
                return false;
            }

            for (u64 i {1}; i <= len; ++i) {
                view.raw_get(idx, static_cast<i64>(i));
                retValue = converter<value_type>::IsType(view, -1);
                view.pop(1);
                if (!retValue) {
                    break;
                }
            }
        }
        return retValue;
    }
};

template <typename T>
struct converter<std::span<T>> {
    void static To(state_view view, std::span<T> const& value)
    {
        view.create_table(0, static_cast<i32>(value.size()));

        for (usize i {0}; i < value.size(); ++i) {
            converter<T>::To(view, value[i]);
            view.raw_set(-2, static_cast<i64>(i + 1));
        }
    }
};

template <>
struct converter<std::filesystem::path> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(state_view view, i32& idx, std::filesystem::path& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.to_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, std::filesystem::path const& value)
    {
        view.push_string(value.string());
    }
};

////lua///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <>
struct converter<ref> {
    auto static IsType(state_view, i32) -> bool
    {
        return true;
    }

    auto static From(state_view view, i32& idx, ref& value) -> bool
    {
        value.acquire(view, idx++);
        return value.is_valid();
    }
};

template <>
struct converter<table> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.is_table(idx);
    }

    auto static From(state_view view, i32& idx, table& value) -> bool
    {
        if (view.is_table(idx)) {
            value.acquire(view, idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view /*view*/, table const& value)
    {
        value.push_self();
    }

    void static To(state_view view, table& value)
    {
        if (!value.is_valid()) {
            value = table::PushNew(view);
        } else {
            value.push_self();
        }
    }
};

template <>
struct converter<coroutine> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.is_thread(idx);
    }

    auto static From(state_view view, i32& idx, coroutine& value) -> bool
    {
        if (view.is_thread(idx)) {
            value.acquire(view, idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view /*view*/, coroutine const& value)
    {
        value.push_self();
    }
};

template <typename T>
struct converter<function<T>> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.is_function(idx);
    }

    auto static From(state_view view, i32& idx, function<T>& value) -> bool
    {
        if (view.is_function(idx)) {
            value.acquire(view, idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view /*view*/, function<T> const& value)
    {
        value.push_self();
    }
};

////basic/////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <>
struct converter<char const*> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(state_view view, i32& idx, char const*& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.to_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, char const* value)
    {
        view.push_string(value);
    }
};

template <usize N>
struct converter<char const[N]> { // NOLINT
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    void static To(state_view view, char const* value)
    {
        view.push_string(value);
    }
};

template <>
struct converter<string> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(state_view view, i32& idx, string& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.to_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, string const& value)
    {
        view.push_string(value);
    }
};

template <>
struct converter<string_view> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(state_view view, i32& idx, string_view& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.to_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, string_view value)
    {
        view.push_lstring(value);
    }
};

template <>
struct converter<std::nullptr_t> {
    void static To(state_view view, std::nullptr_t const&)
    {
        view.push_nil();
    }
};

template <>
struct converter<bool> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::Boolean;
    }

    auto static From(state_view view, i32& idx, bool& value) -> bool
    {
        if (view.is_bool(idx)) {
            value = static_cast<bool>(view.to_bool(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, bool const& value)
    {
        view.push_bool(value);
    }
};

template <Enum T>
struct converter<T> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.is_integer(idx) || view.is_string(idx);
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        if (view.is_integer(idx)) {
            value = static_cast<T>(view.to_integer(idx++));
            return true;
        }

        if (view.is_string(idx)) {
            value = tcob::detail::magic_enum_reduced::string_to_enum<T>(view.to_string(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, T const& value)
    {
        view.push_lstring(tcob::detail::magic_enum_reduced::enum_to_string(value));
    }
};

template <Integral T>
struct converter<T> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.is_integer(idx);
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        if (view.is_integer(idx)) {
            value = static_cast<T>(view.to_integer(idx++));
            return true;
        }
        if (view.is_number(idx)) {
            f64 const val {view.to_number(idx++)};
            if (std::ceil(val) == val) {
                value = static_cast<T>(val);
                return true;
            }

            return false;
        }

        idx++;
        return false;
    }

    void static To(state_view view, T const& value)
    {
        view.push_integer(static_cast<i64>(value));
    }
};

template <FloatingPoint T>
struct converter<T> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::Number;
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        if (view.is_number(idx)) {
            value = static_cast<T>(view.to_number(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, T const& value)
    {
        view.push_number(value);
    }
};

template <Pointer T>
struct converter<T> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.is_userdata(idx);
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        static char const* TypeName {typeid(std::remove_pointer_t<T>).name()};

        if (view.is_userdata(idx)) {
            // try uservalue
            [[maybe_unused]] i32 const err {view.get_uservalue(idx, 1)};
            assert(err != 0);
            string const userDataType {view.to_string(-1)};
            view.pop(1);

            void* ptr {nullptr};

            if (userDataType == TypeName) {
                ptr = view.to_userdata(idx++);
            } else {
                // try metatable
                view.get_metatable(userDataType);
                table tab {table::Acquire(view, -1)};
                view.pop(1);
                if (tab.is_valid()) {
                    if (std::unordered_set<string> types; tab.try_get(types, "__types") && types.contains(TypeName)) {
                        ptr = view.to_userdata(idx++);
                    }
                }
            }

            if (ptr) {
                value = *static_cast<T*>(ptr); // Lua userdata is T**, so dereference it here
                return true;
            }

            value = nullptr;
            return false;
        }

        if constexpr (std::is_same_v<T, char const*>) {
            if (view.is_string(idx)) {
                value = view.to_string(idx++);
                return true;
            }
        } else {
            idx++;
            return false;
        }
    }

    void static To(state_view view, T const& value)
    {
        static char const* TypeName {typeid(std::remove_pointer_t<T>).name()};

        T* obj {static_cast<T*>(view.new_userdata(sizeof(T*), 1))};
        *obj = value;

        view.push_string(TypeName);
        [[maybe_unused]] i32 const err {view.set_uservalue(-2, 1)};
        assert(err != 0);

        view.new_metatable(TypeName);
        view.set_metatable(-2);
    }
};

////tcob//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct converter<scripting::owned_ptr<T>> {
    void static To(state_view view, scripting::owned_ptr<T> const& value)
    {
        static string TypeName {typeid(T).name()};

        T** obj {static_cast<T**>(view.new_userdata(sizeof(T*), 1))};
        *obj = value.Pointer;

        view.push_string(TypeName);
        if (view.set_uservalue(-2, 1) != 0) {
            if (view.new_metatable((TypeName + "_gc")) == 0) {
                // GC table exists
                view.set_metatable(-2);
            } else {
                // create GC metamethod
                if constexpr (std::is_destructible_v<T>) {
                    i32 const tableIdx {view.get_top()};
                    view.push_convert("__gc");
                    view.push_cfunction(&gc);
                    view.set_table(tableIdx);
                }
                view.set_metatable(-2);
            }
        }
    }

    auto static gc(lua_State* l) -> i32
    {
        T** obj {static_cast<T**>(state_view {l}.to_userdata(-1))};

        if (obj && *obj) {
            delete (*obj); // NOLINT
        }

        return 0;
    }
};

template <typename T>
struct converter<result<T>> {
    static consteval auto StackSize() -> i32
    {
        return get_stacksize<T>();
    }

    auto static IsType(state_view view, i32 idx) -> bool
    {
        return converter<T>::IsType(view, idx);
    }

    auto static From(state_view view, i32& idx, result<T>& value) -> bool
    {
        T val;
        if (converter<T>::From(view, idx, val)) {
            value = result<T> {std::move(val)};
            return true;
        }

        value = result<T> {error_code::TypeMismatch};
        return false;
    }

    void static To(state_view view, result<T> const& value)
    {
        converter<T>::To(view, value.value());
    }
};

template <typename... Keys>
struct converter<proxy<table, Keys...>> {
    void static To(state_view, proxy<table, Keys...> const& value)
    {
        if (ref val; value.try_get(val)) {
            val.push_self();
        }
    }
};

template <typename... Keys>
struct converter<proxy<table const, Keys...>> {
    void static To(state_view, proxy<table const, Keys...> const& value)
    {
        if (ref val; value.try_get(val)) {
            val.push_self();
        }
    }
};

template <Serializable<table> T>
struct converter<T> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        if (view.is_table(idx)) {
            table tab {table::Acquire(view, idx)};
            T     t {};
            return T::Deserialize(t, tab);
        }

        return false;
    }

    auto static From(state_view view, i32& idx, T& value) -> bool
    {
        table tab {table::Acquire(view, idx++)};
        return T::Deserialize(value, tab);
    }

    void static To(state_view view, T const& value)
    {
        table tab {table::PushNew(view)};
        T::Serialize(value, tab);
    }
};

template <FloatingPoint T>
struct converter<degree<T>> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::Number;
    }

    auto static From(state_view view, i32& idx, degree<T>& value) -> bool
    {
        if (view.is_number(idx)) {
            value = static_cast<T>(view.to_number(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, degree<T> const& value)
    {
        view.push_number(value.Value);
    }
};

template <FloatingPoint T>
struct converter<radian<T>> {
    auto static IsType(state_view view, i32 idx) -> bool
    {
        return view.get_type(idx) == type::Number;
    }

    auto static From(state_view view, i32& idx, radian<T>& value) -> bool
    {
        if (view.is_number(idx)) {
            value = static_cast<T>(view.to_number(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(state_view view, radian<T> const& value)
    {
        view.push_number(value.Value);
    }
};

static_assert(get_stacksize<i32>() == 1);
static_assert(get_stacksize<std::optional<i32>>() == 1);
static_assert(get_stacksize<std::pair<f32, i32>>() == 2);
static_assert(get_stacksize<std::optional<std::pair<f32, i32>>>() == 2);
static_assert(get_stacksize<std::tuple<f32, i32, bool>>() == 3);
static_assert(get_stacksize<std::optional<std::tuple<f32, i32, bool>>>() == 3);

}

#endif
