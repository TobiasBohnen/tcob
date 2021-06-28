// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <functional>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <tcob/core/data/Color.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/script/LuaClosure.hpp>
#include <tcob/script/LuaFunction.hpp>
#include <tcob/script/LuaState.hpp>
#include <tcob/script/LuaTable.hpp>
#include <tcob/script/LuaWrapper.hpp>

namespace tcob {
template <typename T>
struct LuaConverter {
};

template <typename R, typename... P>
struct LuaConverter<R(P...)> {
    static constexpr i32 StackSlots { 1 };

    static void ToLua(const LuaState& ls, R (*value)(P...))
    {
        ls.push_lightuserdata(reinterpret_cast<void*>(value));
        ls.push_cclosure(
            [](lua_State* l) -> i32 {
                auto p { reinterpret_cast<R(*)(P...)>(LuaState { l }.to_userdata(LuaState::UpvalueIndex(1))) };
                std::function<R(P...)> func { p };
                detail::LuaClosure<R(P...)> cl { func };
                return cl.invoke(l); },
            1);
    }
};

template <typename R, typename... P>
struct LuaConverter<std::function<R(P...)>> {
    static constexpr i32 StackSlots { 1 };

    static void ToLua(const LuaState& ls, std::function<R(P...)>& value)
    {
        ls.push_lightuserdata(reinterpret_cast<void*>(&value));
        ls.push_cclosure(
            [](lua_State* l) -> i32 {
                auto* p { reinterpret_cast<std::function<R(P...)>*>(LuaState { l }.to_userdata(LuaState::UpvalueIndex(1)))};
                detail::LuaClosure<R(P...)> cl { *p };
                return cl.invoke(l); },
            1);
    }
};

template <>
struct LuaConverter<detail::LuaClosureBase*> {
    static constexpr i32 StackSlots { 1 };

    static void ToLua(const LuaState& ls, detail::LuaClosureBase* value)
    {
        ls.push_lightuserdata(reinterpret_cast<void*>(value));
        ls.push_cclosure(
            [](lua_State* l) -> i32 {                
                auto* p { reinterpret_cast<detail::LuaClosureBase*>(LuaState { l }.to_userdata(LuaState::UpvalueIndex(1))) };
                return (*p)(l); },
            1);
    }
};

template <typename T>
struct LuaConverter<std::optional<T>> {
    static constexpr i32 StackSlots { LuaConverter<T>::StackSlots };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return LuaConverter<T>::IsType(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::optional<T>& value) -> bool
    {
        i32 top { ls.get_top() };
        if (idx > top || !LuaConverter<T>::IsType(ls, idx)) {
            value = std::nullopt;
            return false;
        } else {
            T val {};
            LuaConverter<T>::FromLua(ls, std::forward<i32>(idx), val);
            value = val;
            return true;
        }
    }
};

template <typename T>
struct LuaConverter<LuaOwnedPtr<T>> {
    static constexpr i32 StackSlots { 1 };

    static void ToLua(const LuaState& ls, const LuaOwnedPtr<T>& value)
    {
        T** obj { static_cast<T**>(ls.new_userdata(sizeof(T*), 1)) };
        *obj = value.Obj;

        ls.push_string(TypeName.c_str());
        assert(ls.set_uservalue(-2, 1) != 0);

        if (ls.new_metatable((TypeName + "_gc").c_str()) == 0) {
            // GC table exists
            ls.set_metatable(-2);
        } else {
            // create GC metamethod
            if constexpr (std::is_destructible_v<T>) {
                const i32 tableIdx { ls.get_top() };
                ls.push("__gc");
                ls.push_cfunction(&gc);
                ls.set_table(tableIdx);
            }
            ls.set_metatable(-2);
        }
    }

    static auto gc(lua_State* l) -> i32
    {
        T** obj { static_cast<T**>(LuaState { l }.to_userdata(-1)) };

        if (obj && *obj)
            delete (*obj);

        return 0;
    }

    static inline std::string TypeName { typeid(T).name() };
};

template <typename T>
struct LuaConverter<LuaResult<T>> {
    static constexpr i32 StackSlots { LuaConverter<T>::StackSlots };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return LuaConverter<T>::IsType(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, LuaResult<T>& value) -> bool
    {
        bool ok { LuaConverter<T>::FromLua(ls, std::forward<i32>(idx), value.Value) };
        value.State = ok ? LuaResultState::Ok : LuaResultState::TypeMismatch;
        return ok;
    }

    static void ToLua(const LuaState& ls, const LuaResult<T>& value)
    {
        LuaConverter<T>::ToLua(ls, value.Value);
    }
};

template <typename... P>
struct LuaConverter<std::variant<P...>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return check_variant<P...>(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::variant<P...>& value) -> bool
    {
        switch (ls.get_type(idx)) {
        case LuaType::Number:
            if (ls.is_integer(idx)) {
                if constexpr (contains<i32, P...>()) {
                    value = static_cast<i32>(ls.to_integer(idx++));
                } else if constexpr (contains<i64, P...>()) {
                    value = static_cast<i64>(ls.to_integer(idx++));
                } else if constexpr (contains<i16, P...>()) {
                    value = static_cast<i16>(ls.to_integer(idx++));
                } else if constexpr (contains<i8, P...>()) {
                    value = static_cast<i8>(ls.to_integer(idx++));
                } else if constexpr (contains<u32, P...>()) {
                    value = static_cast<u32>(ls.to_integer(idx++));
                } else if constexpr (contains<u64, P...>()) {
                    value = static_cast<u64>(ls.to_integer(idx++));
                } else if constexpr (contains<u16, P...>()) {
                    value = static_cast<u16>(ls.to_integer(idx++));
                } else if constexpr (contains<u8, P...>()) {
                    value = static_cast<u8>(ls.to_integer(idx++));
                }
            } else if constexpr (contains<f32, P...>()) {
                value = static_cast<f32>(ls.to_number(idx++));
            } else if constexpr (contains<f64, P...>()) {
                value = static_cast<f64>(ls.to_number(idx++));
            }
            break;
        case LuaType::String:
            if constexpr (contains<std::string, P...>()) {
                value = std::string(ls.to_string(idx++));
            }
            break;
        case LuaType::Boolean:
            if constexpr (contains<bool, P...>()) {
                value = static_cast<bool>(ls.to_bool(idx++));
            }
            break;
        case LuaType::Table: {
            // TODO: more types
            if constexpr (contains<Color, P...>()) {
                Color c;
                if (from_lua(ls, idx, c)) {
                    value = c;
                    break;
                }
            }

            if constexpr ((detail::is_specialization<P, std::vector>() || ...)) {
                if (get_specialization<std::vector, P...>(ls, idx, value))
                    break;
            }
            if constexpr ((detail::is_specialization<P, Point>() || ...)) {
                if (get_specialization<Point, P...>(ls, idx, value))
                    break;
            }
            if constexpr ((detail::is_specialization<P, Size>() || ...)) {
                if (get_specialization<Size, P...>(ls, idx, value))
                    break;
            }
            if constexpr ((detail::is_specialization<P, Rect>() || ...)) {
                if (get_specialization<Rect, P...>(ls, idx, value))
                    break;
            }
        } break;
        default:
            idx++;
            return false;
        }

        return true;
    }

    static void ToLua(const LuaState& ls, const std::variant<P...>& value)
    {
        std::visit(
            [&ls](auto&& item) {
                to_lua(ls, item);
            },
            value);
    }

private:
    template <typename T, typename... Ts>
    static constexpr auto contains() -> bool
    {
        return std::disjunction_v<std::is_same<T, Ts>...>;
    }

    template <typename R>
    static auto from_lua(const LuaState& ls, i32 idx, R& value) -> bool
    {
        return LuaConverter<R>::FromLua(ls, std::forward<i32>(idx), value);
    }

    template <typename R>
    static void to_lua(const LuaState& ls, const R& value)
    {
        LuaConverter<R>::ToLua(ls, value);
    }

    template <template <typename...> typename C, typename T, typename... Ts>
    static auto get_specialization(const LuaState& ls, i32 idx, std::variant<P...>& value) -> bool
    {
        if constexpr (sizeof...(Ts) > 0) {
            if constexpr (detail::is_specialization<T, C>()) {
                T vec;
                if (from_lua(ls, idx, vec)) {
                    value = vec;
                    return true;
                }
            }

            return get_specialization<C, Ts...>(ls, idx, value);
        } else {
            return false;
        }
    }

    template <typename T, typename... Ts>
    static auto check_variant(const LuaState& ls, i32 idx) -> bool
    {
        if constexpr (sizeof...(Ts) > 0) {
            return LuaConverter<T>::IsType(ls, idx) || check_variant<Ts...>(ls, idx);
        } else {
            return LuaConverter<T>::IsType(ls, idx);
        }
    }
};

template <typename K, typename V>
struct LuaConverter<std::map<K, V>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return check_map(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::map<K, V>& value) -> bool
    {
        if (ls.is_table(idx)) {
            bool retValue { true };

            ls.push_value(idx); // stack: -1 => table
            ls.push_nil(); // stack: -1 => nil; -2 => table

            while (ls.next(-2)) { // stack: -1 => value; -2 => key; -3 => table
                ls.push_value(-2); // stack: -1 => key; -2 => value; -3 => key; -4 => table
                K key {};
                retValue &= LuaConverter<K>::FromLua(ls, -1, key);
                retValue &= LuaConverter<V>::FromLua(ls, -2, value[key]);
                ls.pop(2); // stack: -1 => key; -2 => table
            }

            ls.pop(1); // stack: -1 => table

            idx++;
            return retValue;
        }
        return false;
    }

    static void ToLua(const LuaState& ls, const std::map<K, V>& value)
    {
        ls.create_table(0, static_cast<i32>(value.size()));
        for (auto& [key, val] : value) {
            LuaConverter<K>::ToLua(ls, key);
            LuaConverter<V>::ToLua(ls, val);
            ls.set_table(-3);
        }
    }

private:
    static auto check_map(const LuaState& ls, i32 idx) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            ls.push_value(idx); // stack: -1 => table
            ls.push_nil(); // stack: -1 => nil; -2 => table

            while (ls.next(-2)) { // stack: -1 => value; -2 => key; -3 => table
                ls.push_value(-2); // stack: -1 => key; -2 => value; -3 => key; -4 => table
                retValue = LuaConverter<K>::IsType(ls, -1) && LuaConverter<V>::IsType(ls, -2);
                ls.pop(2); // stack: -1 => key; -2 => table

                if (!retValue) {
                    break;
                }
            }

            ls.pop(1); // stack: -1 => table
        }

        return retValue;
    }
};

template <typename K, typename V>
struct LuaConverter<std::unordered_map<K, V>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return check_map(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::unordered_map<K, V>& value) -> bool
    {
        if (ls.is_table(idx)) {
            bool retValue { true };

            ls.push_value(idx); // stack: -1 => table
            ls.push_nil(); // stack: -1 => nil; -2 => table

            while (ls.next(-2)) { // stack: -1 => value; -2 => key; -3 => table
                ls.push_value(-2); // stack: -1 => key; -2 => value; -3 => key; -4 => table
                K key {};
                retValue &= LuaConverter<K>::FromLua(ls, -1, key);
                retValue &= LuaConverter<V>::FromLua(ls, -2, value[key]);
                ls.pop(2); // stack: -1 => key; -2 => table
            }

            ls.pop(1); // stack: -1 => table

            idx++;
            return retValue;
        }
        return false;
    }

    static void ToLua(const LuaState& ls, const std::unordered_map<K, V>& value)
    {
        ls.create_table(0, static_cast<i32>(value.size()));
        for (auto& [key, val] : value) {
            LuaConverter<K>::ToLua(ls, key);
            LuaConverter<V>::ToLua(ls, val);
            ls.set_table(-3);
        }
    }

private:
    static auto check_map(const LuaState& ls, i32 idx) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            ls.push_value(idx); // stack: -1 => table
            ls.push_nil(); // stack: -1 => nil; -2 => table

            while (ls.next(-2)) { // stack: -1 => value; -2 => key; -3 => table
                ls.push_value(-2); // stack: -1 => key; -2 => value; -3 => key; -4 => table
                retValue = LuaConverter<K>::IsType(ls, -1) && LuaConverter<V>::IsType(ls, -2);
                ls.pop(2); // stack: -1 => key; -2 => table

                if (!retValue) {
                    break;
                }
            }

            ls.pop(1); // stack: -1 => table
        }

        return retValue;
    }
};

template <typename T>
struct LuaConverter<std::set<T>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return check_set(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::set<T>& value) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            const auto len { ls.raw_len(idx) };
            for (isize i { 1 }; i <= len; ++i) {
                ls.raw_get(idx, i);
                T val {};
                retValue = LuaConverter<T>::FromLua(ls, -1, val);
                ls.pop(1);
                if (retValue) {
                    if (!value.contains(val)) {
                        value.insert(val);
                    } else {
                        retValue = false;
                        break;
                    }
                } else {
                    break;
                }
            }
            idx++;
        }
        return retValue;
    }

    static void ToLua(const LuaState& ls, const std::set<T>& value)
    {
        ls.create_table(0, static_cast<i32>(value.size()));

        i32 i { 0 };
        for (auto& val : value) {
            LuaConverter<T>::ToLua(ls, val);
            ls.raw_set(-2, ++i);
        }
    }

private:
    static auto check_set(const LuaState& ls, i32 idx) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            const auto len { ls.raw_len(idx) };

            for (isize i { 1 }; i <= len; ++i) {
                ls.raw_get(idx, i);
                retValue = LuaConverter<T>::IsType(ls, -1);
                ls.pop(1);
                if (!retValue) {
                    break;
                }
            }
        }
        return retValue;
    }
};

template <typename T>
struct LuaConverter<std::unordered_set<T>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return check_set(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::unordered_set<T>& value) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            const auto len { ls.raw_len(idx) };
            for (isize i { 1 }; i <= len; ++i) {
                ls.raw_get(idx, i);
                T val {};
                retValue = LuaConverter<T>::FromLua(ls, -1, val);
                ls.pop(1);
                if (retValue) {
                    if (!value.contains(val)) {
                        value.insert(val);
                    } else {
                        retValue = false;
                        break;
                    }
                } else {
                    break;
                }
            }
            idx++;
        }
        return retValue;
    }

    static void ToLua(const LuaState& ls, const std::unordered_set<T>& value)
    {
        ls.create_table(0, static_cast<i32>(value.size()));

        i32 i { 0 };
        for (auto& val : value) {
            LuaConverter<T>::ToLua(ls, val);
            ls.raw_set(-2, ++i);
        }
    }

private:
    static auto check_set(const LuaState& ls, i32 idx) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            const auto len { ls.raw_len(idx) };

            for (isize i { 1 }; i <= len; ++i) {
                ls.raw_get(idx, i);
                retValue = LuaConverter<T>::IsType(ls, -1);
                ls.pop(1);
                if (!retValue) {
                    break;
                }
            }
        }
        return retValue;
    }
};

template <typename... T>
struct LuaConverter<std::tuple<T...>> {
    static constexpr i32 StackSlots { static_cast<i32>(std::tuple_size_v<std::tuple<T...>>) };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return (LuaConverter<T>::IsType(ls, idx++) && ...);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::tuple<T...>& value) -> bool
    {
        bool retValue { true };
        std::apply(
            [ls, &idx, &retValue](auto&&... item) {
                ((retValue &= from_lua(ls, std::forward<i32>(idx), item)) && ...);
            },
            value);

        return retValue;
    }

    static void ToLua(const LuaState& ls, const std::tuple<T...>& value)
    {
        std::apply([ls](auto&&... item) {
            (to_lua(ls, item), ...);
        },
            value);
    }

private:
    template <typename R>
    static auto from_lua(const LuaState& ls, i32&& idx, R& value) -> bool
    {
        return LuaConverter<R>::FromLua(ls, std::forward<i32>(idx), value);
    }

    template <typename R>
    static void to_lua(const LuaState& ls, const R& value)
    {
        LuaConverter<R>::ToLua(ls, value);
    }
};

template <typename T, isize Size>
struct LuaConverter<std::array<T, Size>> {
    static constexpr i32 StackSlots { 1 };

    static auto FromLua(const LuaState& ls, i32&& idx, std::array<T, Size>& value) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            isize len = Size;

            for (isize i = 1; i <= len; ++i) {
                ls.raw_get(idx, i);
                retValue &= LuaConverter<T>::FromLua(ls, -1, value[i - 1]);
                ls.pop(1);
            }

            idx++;
        }
        return retValue;
    }

    static void ToLua(const LuaState& ls, const std::array<T, Size>& value)
    {
        ls.create_table(0, static_cast<i32>(Size));

        for (isize i { 0 }; i < Size; ++i) {
            LuaConverter<T>::ToLua(ls, value[i]);
            ls.raw_set(-2, i + 1);
        }
    }
};

template <typename T>
struct LuaConverter<std::vector<T>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return check_vector(ls, idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::vector<T>& value) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            const auto len { ls.raw_len(idx) };
            value.resize(len);

            for (isize i { 1 }; i <= len; ++i) {
                ls.raw_get(idx, i);
                T val {};
                retValue = LuaConverter<T>::FromLua(ls, -1, val);
                ls.pop(1);
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

    static void ToLua(const LuaState& ls, const std::vector<T>& value)
    {
        ls.create_table(0, static_cast<i32>(value.size()));

        for (isize i { 0 }; i < value.size(); ++i) {
            LuaConverter<T>::ToLua(ls, value[i]);
            ls.raw_set(-2, i + 1);
        }
    }

private:
    static auto check_vector(const LuaState& ls, i32 idx) -> bool
    {
        bool retValue { ls.is_table(idx) };
        if (retValue) {
            const auto len { ls.raw_len(idx) };

            for (isize i { 1 }; i <= len; ++i) {
                ls.raw_get(idx, i);
                retValue = LuaConverter<T>::IsType(ls, -1);
                ls.pop(1);
                if (!retValue) {
                    break;
                }
            }
        }
        return retValue;
    }
};

template <typename K, typename V>
struct LuaConverter<std::pair<K, V>> {
    static constexpr i32 StackSlots { 2 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return LuaConverter<K>::IsType(ls, idx) && LuaConverter<V>::IsType(ls, idx + 1);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::pair<K, V>& value) -> bool
    {
        bool retValue;
        K first {};
        retValue = LuaConverter<K>::FromLua(ls, std::forward<i32>(idx), first);
        V second {};
        retValue &= LuaConverter<V>::FromLua(ls, std::forward<i32>(idx), second);
        if (retValue) {
            auto pair = std::make_pair(first, second);
            value.swap(pair);
        }
        return retValue;
    }

    static void ToLua(const LuaState& ls, const std::pair<K, V>& value)
    {
        LuaConverter<K>::ToLua(ls, value.first);
        LuaConverter<V>::ToLua(ls, value.second);
    }
};

template <>
struct LuaConverter<LuaTable> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.is_table(idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, LuaTable& value) -> bool
    {
        if (ls.is_table(idx)) {
            value.ref(ls, idx);
            idx++;
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState&, const LuaTable& value)
    {
        value.push_self();
    }

    static void ToLua(const LuaState& ls, LuaTable& value)
    {
        if (!value.is_valid()) {
            ls.new_table();
            value.ref(ls, -1);
            ls.pop(1);
        }

        value.push_self();
    }
};

template <>
struct LuaConverter<std::string> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.get_type(idx) == LuaType::String;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, std::string& value) -> bool
    {
        if (ls.is_string(idx)) {
            value = ls.to_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState& ls, const std::string& value)
    {
        ls.push_string(value.c_str());
    }
};

template <>
struct LuaConverter<std::nullptr_t> {
    static constexpr i32 StackSlots { 1 };

    static void ToLua(const LuaState& ls, const std::nullptr_t&)
    {
        ls.push_nil();
    }
};

template <>
struct LuaConverter<bool> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.get_type(idx) == LuaType::Boolean;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, bool& value) -> bool
    {
        if (ls.is_bool(idx)) {
            value = static_cast<bool>(ls.to_bool(idx++));
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState& ls, const bool& value)
    {
        ls.push_bool(value);
    }
};

template <>
struct LuaConverter<LuaCoroutine> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.is_thread(idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, LuaCoroutine& value) -> bool
    {
        if (ls.is_thread(idx)) {
            value.ref(ls, idx);
            idx++;
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState&, const LuaCoroutine& value)
    {
        value.push_self();
    }
};

template <typename T>
struct LuaConverter<LuaFunction<T>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.is_function(idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, LuaFunction<T>& value) -> bool
    {
        if (ls.is_function(idx)) {
            value.ref(ls, idx);
            idx++;
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState& ls, const LuaFunction<T>& value)
    {
        value.push_self();
    }
};

template <tcob::Enum T>
struct LuaConverter<T> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.is_integer(idx) == 1;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, T& value) -> bool
    {
        if (ls.is_integer(idx)) {
            value = static_cast<T>(ls.to_integer(idx++));
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState& ls, const T& value)
    {
        ls.push_integer(static_cast<i32>(value));
    }
};

template <tcob::Integral T>
struct LuaConverter<T> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.is_integer(idx) == 1;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, T& value) -> bool
    {
        if (ls.is_integer(idx)) {
            value = static_cast<T>(ls.to_integer(idx++));
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState& ls, const T& value)
    {
        ls.push_integer(value);
    }
};

template <tcob::FloatingPoint T>
struct LuaConverter<T> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.get_type(idx) == LuaType::Number;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, T& value) -> bool
    {
        if (ls.is_number(idx)) {
            value = static_cast<T>(ls.to_number(idx++));
            return true;
        }

        idx++;
        return false;
    }

    static void ToLua(const LuaState& ls, const T& value)
    {
        ls.push_number(value);
    }
};

template <tcob::Pointer T>
struct LuaConverter<T> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        return ls.is_userdata(idx);
    }

    static auto FromLua(const LuaState& ls, i32&& idx, T& value) -> bool
    {
        if (ls.is_userdata(idx)) {
            assert(ls.get_uservalue(idx, 1) != 0);
            std::string userdatatype { ls.to_string(-1) };
            ls.pop(1);
            if (TypeName == userdatatype) {
                value = *static_cast<T*>(ls.to_userdata(idx++));
                return true;
            } else {
                value = nullptr;
                return false;
            }

        } else if constexpr (std::is_same_v<T, const char*>) {
            if (ls.is_string(idx)) {
                value = ls.to_string(idx++);
                return true;
            }
        }
        idx++;
        return false;
    }

    static void ToLua(const LuaState& ls, const T& value)
    {
        T* obj { static_cast<T*>(ls.new_userdata(sizeof(T*), 1)) };
        *obj = value;

        ls.push_string(TypeName.c_str());
        assert(ls.set_uservalue(-2, 1) != 0);

        ls.new_metatable(TypeName.c_str());
        ls.set_metatable(-2);
    }

    static inline std::string TypeName { typeid(std::remove_pointer_t<T>).name() };
};

template <>
struct LuaConverter<Color> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx };
            return (lt.has("r") && lt.has("g") && lt.has("b"));
        }
        return false;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, Color& value) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx++ };

            if (lt.has("r") && lt.has("g") && lt.has("b")) {
                value.R = lt["r"];
                value.G = lt["g"];
                value.B = lt["b"];
                value.A = lt.has("a") ? lt["a"] : static_cast<u8>(255);
                return true;
            }
        }
        return false;
    }

    static void ToLua(const LuaState& ls, const Color& value)
    {
        ls.new_table();
        LuaTable lt { ls, -1 };

        lt["r"] = value.R;
        lt["g"] = value.G;
        lt["b"] = value.B;
        lt["a"] = value.A;
    }
};

template <typename T>
struct LuaConverter<Point<T>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx };
            return (lt.has("x") && lt.has("y")) || lt.raw_length() == 2;
        }
        return false;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, Point<T>& value) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx++ };
            if (lt.has("x") && lt.has("y")) {
                value.X = lt["x"];
                value.Y = lt["y"];
                return true;
            } else if (lt.raw_length() == 2) {
                value.X = lt[1];
                value.Y = lt[2];
                return true;
            }
        }

        return false;
    }

    static void ToLua(const LuaState& ls, const Point<T>& value)
    {
        ls.new_table();
        LuaTable lt { ls, -1 };

        lt["x"] = value.X;
        lt["y"] = value.Y;
    }
};

template <typename T>
struct LuaConverter<Size<T>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx };
            return (lt.has("width") && lt.has("height")) || lt.raw_length() == 2;
        }
        return false;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, Size<T>& value) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx++ };
            if (lt.has("width") && lt.has("height")) {
                value.Width = lt["width"];
                value.Height = lt["height"];
                return true;
            } else if (lt.raw_length() == 2) {
                value.Width = lt[1];
                value.Height = lt[2];
                return true;
            }
        }

        return false;
    }

    static void ToLua(const LuaState& ls, const Size<T>& value)
    {
        ls.new_table();
        LuaTable lt { ls, -1 };

        lt["width"] = value.Width;
        lt["height"] = value.Height;
    }
};

template <typename T>
struct LuaConverter<Rect<T>> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx };
            return (lt.has("left") && lt.has("top") && lt.has("width") && lt.has("height")) || lt.raw_length() == 4;
        }
        return false;
    }

    static auto FromLua(const LuaState& ls, i32&& idx, Rect<T>& value) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx++ };
            if (lt.has("left") && lt.has("top") && lt.has("width") && lt.has("height")) {
                value.Left = lt["left"];
                value.Top = lt["top"];
                value.Width = lt["width"];
                value.Height = lt["height"];
                return true;
            } else if (lt.raw_length() == 4) {
                value.Left = lt[1];
                value.Top = lt[2];
                value.Width = lt[3];
                value.Height = lt[4];
                return true;
            }
        }

        return false;
    }

    static void ToLua(const LuaState& ls, const Rect<T>& value)
    {
        ls.new_table();
        LuaTable lt { ls, -1 };

        lt["left"] = value.Left;
        lt["top"] = value.Top;
        lt["width"] = value.Width;
        lt["height"] = value.Height;
        lt["center"] = value.center();
    }
};
}