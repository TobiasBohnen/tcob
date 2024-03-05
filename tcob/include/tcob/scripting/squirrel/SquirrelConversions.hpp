// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// IWYU pragma: always_keep

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <filesystem>
    #include <functional>
    #include <optional>
    #include <span>
    #include <type_traits>
    #include <utility>
    #include <variant>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Concepts.hpp"
    #include "tcob/scripting/Wrapper.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"
    #include "tcob/scripting/squirrel/SquirrelClosure.hpp"
    #include "tcob/scripting/squirrel/SquirrelTypes.hpp"

    #include "tcob/core/ext/magic_enum_reduced.hpp"

namespace tcob::scripting::squirrel {

////functions/////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename R, typename... P>
struct converter<R(P...)> {
    void static To(vm_view view, R (*value)(P...))
    {
        view.push_userpointer(reinterpret_cast<void*>(value));
        view.new_closure(
            [](HSQUIRRELVM l) -> SQInteger {
                vm_view s {l};
                void*   ptr {nullptr};
                s.get_userpointer(-1, &ptr);
                detail::native_closure<R(P...)> cl {reinterpret_cast<R (*)(P...)>(ptr)};
                return cl(s);
            },
            1);
    }
};

template <typename R, typename... P>
struct converter<std::function<R(P...)>*> {
    void static To(vm_view view, std::function<R(P...)>* value)
    {
        view.push_userpointer(reinterpret_cast<void*>(value));
        view.new_closure(
            [](HSQUIRRELVM l) -> SQInteger {
                vm_view s {l};
                void*   ptr {nullptr};
                s.get_userpointer(-1, &ptr);
                detail::native_closure<R(P...)> cl {*reinterpret_cast<std::function<R(P...)>*>(ptr)};
                return cl(s);
            },
            1);
    }
};

template <>
struct converter<detail::native_closure_base*> {
    void static To(vm_view view, detail::native_closure_base* value)
    {
        view.push_userpointer(reinterpret_cast<void*>(value));
        view.new_closure(
            [](HSQUIRRELVM l) -> SQInteger {
                vm_view s {l};
                void*   ptr {nullptr};
                s.get_userpointer(-1, &ptr);
                auto* p {reinterpret_cast<detail::native_closure_base*>(ptr)};
                return (*p)(s);
            },
            1);
    }
};

////STL///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct converter<std::optional<T>> {
    auto static IsType(vm_view, SQInteger) -> bool
    {
        return true;
    }

    auto static From(vm_view view, SQInteger& idx, std::optional<T>& value) -> bool
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

    void static To(vm_view view, std::optional<T> const& value)
    {
        if (value) {
            converter<T>::To(view, *value);
        } else {
            view.push_null();
        }
    }
};

template <>
struct converter<std::nullopt_t> {
    void static To(vm_view view, std::nullopt_t const&)
    {
        view.push_null();
    }
};

template <typename... P>
struct converter<std::variant<P...>> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return check_variant<P...>(view, idx);
    }

    auto static From(vm_view view, SQInteger& idx, std::variant<P...>& value) -> bool
    {
        return convert_from<P...>(view, idx, value);
    }

    void static To(vm_view view, std::variant<P...> const& value)
    {
        std::visit([&view](auto&& item) { convert_to(view, item); }, value);
    }

private:
    template <typename T, typename... Ts>
    auto static convert_from(vm_view view, SQInteger& idx, std::variant<P...>& value) -> bool
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
    void static convert_to(vm_view view, R const& value)
    {
        converter<R>::To(view, value);
    }

    template <typename T, typename... Ts>
    auto static check_variant(vm_view view, SQInteger idx) -> bool
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

    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return check_map(view, idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        if (view.is_table(idx) || view.is_array(idx)) {
            bool retValue {true};

            view.push(idx);                     // stack: -1 => table
            view.push_null();                   // stack: -1 => nil; -2 => table

            while (view.next(-2) && retValue) { // stack: -1 => value; -2 => key; -3 => iterator; -4 => table
                key_type  key {};
                SQInteger idx0 {-2}, idx1 {-1};
                retValue = converter<key_type>::From(view, idx0, key)
                    && converter<mapped_type>::From(view, idx1, value[key]);
                view.pop(2); // stack: -1 => iterator; -2 => table
            }

            view.pop(1);     // stack: -1 => table

            idx++;
            return retValue;
        }
        return false;
    }

    void static To(vm_view view, T const& value)
    {
        view.new_table(std::ssize(value));
        for (auto& [key, val] : value) {
            converter<key_type>::To(view, key);
            converter<mapped_type>::To(view, val);
            view.new_slot(-3, false);
        }
    }

private:
    auto static check_map(vm_view view, SQInteger idx) -> bool
    {
        bool retValue {view.is_table(idx) || view.is_array(idx)};
        if (retValue) {
            view.push(idx);                     // stack: -1 => table
            view.push_null();                   // stack: -1 => nil; -2 => table

            while (view.next(-2) && retValue) { // stack: -1 => value; -2 => key; -3 => table
                view.push(-2);                  // stack: -1 => key; -2 => value; -3 => key; -4 => table
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

    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return check_set(view, idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        bool retValue {view.is_array(idx)};
        if (retValue) {
            SQInteger const len {view.get_size(idx)};
            for (SQInteger i {0}; i < len && retValue; ++i) {
                view.push_integer(i);
                view.raw_get(idx);
                key_type  val {};
                SQInteger idx2 {-1};
                retValue = converter<key_type>::From(view, idx2, val);
                value.insert(val);
                view.pop(1);
            }
            idx++;
        }
        return retValue;
    }

    void static To(vm_view view, T const& value)
    {
        view.new_array(std::ssize(value));

        for (SQInteger i {0}; auto const& val : value) {
            view.push_convert(i++);
            converter<key_type>::To(view, val);
            view.set(-3);
        }
    }

private:
    auto static check_set(vm_view view, SQInteger idx) -> bool
    {
        bool retValue {view.is_array(idx)};
        if (retValue) {
            SQInteger const len {view.get_size(idx)};
            for (SQInteger i {0}; i < len && retValue; ++i) {
                view.push_integer(i);
                view.raw_get(idx);
                retValue = converter<key_type>::IsType(view, -1);
                view.pop(1);
            }
        }
        return retValue;
    }
};

template <typename T>
struct converter<parameter_pack<T>> {

    void static To(vm_view view, parameter_pack<T> const& value)
    {
        for (auto const& item : value.Items) {
            converter<T>::To(view, item);
        }
    }
};

template <typename T, usize Size>
struct converter<std::array<T, Size>> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return check_array(view, idx);
    }

    auto static From(vm_view view, SQInteger& idx, std::array<T, Size>& value) -> bool
    {
        SQInteger const size {static_cast<SQInteger>(Size)};
        bool            retValue {view.is_array(idx) && view.get_size(idx) == size};

        for (SQInteger i {0}; auto& val : value) {
            view.push_integer(i++);
            view.raw_get(idx);
            SQInteger idx2 {-1};
            retValue = converter<T>::From(view, idx2, val);
            view.pop(1);
            if (!retValue) {
                break;
            }
        }

        idx++;

        return retValue;
    }

    void static To(vm_view view, std::array<T, Size> const& value)
    {
        view.new_array(static_cast<SQInteger>(Size));

        for (SQInteger i {0}; auto const& val : value) {
            view.push_convert(i++);
            converter<T>::To(view, val);
            view.set(-3);
        }
    }

private:
    auto static check_array(vm_view view, SQInteger idx) -> bool
    {
        SQInteger const size {static_cast<SQInteger>(Size)};
        bool            retValue {view.is_array(idx) && view.get_size(idx) == size};

        for (SQInteger i {0}; i < size && retValue; ++i) {
            view.push_integer(i);
            view.raw_get(idx);
            retValue = converter<T>::IsType(view, -1);
            view.pop(1);
        }
        return retValue;
    }
};

template <Container T>
struct converter<T> {
    using value_type = typename T::value_type;

    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return check_vector(view, idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        bool retValue {view.is_array(idx)};
        if (retValue) {
            SQInteger const len {view.get_size(idx)};
            value.resize(static_cast<usize>(len));

            for (SQInteger i {0}; i < len; ++i) {
                view.push_integer(i);
                view.raw_get(idx);
                value_type val {};
                SQInteger  idx2 {-1};
                retValue = converter<value_type>::From(view, idx2, val);
                view.pop(1);
                if (retValue) {
                    value[static_cast<usize>(i)] = val;
                } else {
                    value.resize(static_cast<usize>(i));
                    break;
                }
            }
            idx++;
        }
        return retValue;
    }

    void static To(vm_view view, T const& value)
    {
        SQInteger const size {std::ssize(value)};
        view.new_array(size);

        for (SQInteger i {0}; i < size; ++i) {
            view.push_convert(i);
            converter<value_type>::To(view, value[static_cast<usize>(i)]);
            view.set(-3);
        }
    }

private:
    auto static check_vector(vm_view view, SQInteger idx) -> bool
    {
        bool retValue {view.is_array(idx)};
        if (retValue) {
            SQInteger const len {view.get_size(idx)};
            if (len == 0) {
                return false;
            }

            for (SQInteger i {0}; i < len; ++i) {
                view.push_integer(i);
                view.raw_get(idx);
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
    void static To(vm_view view, std::span<T> const& value)
    {
        SQInteger const size {std::ssize(value)};
        view.new_array(size);

        for (SQInteger i {0}; i < size; ++i) {
            view.push_integer(i);
            converter<T>::To(view, value[static_cast<usize>(i)]);
            view.set(-3);
        }
    }
};

template <typename K, typename V>
struct converter<std::pair<K, V>> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.is_array(idx) && view.get_size(idx) == 2; // TODO: check types
    }

    auto static From(vm_view view, SQInteger& idx, std::pair<K, V>& value) -> bool
    {
        bool retValue {view.is_array(idx) && view.get_size(idx) == 2};
        if (retValue) {
            SQInteger idx1 {-1};
            view.push_integer(0);
            view.raw_get(idx);
            retValue = converter<K>::From(view, idx1, value.first);
            view.pop(1);

            SQInteger idx2 {-1};
            view.push_integer(1);
            view.raw_get(idx);
            retValue = converter<V>::From(view, idx2, value.second);
            view.pop(1);
        }

        idx++;

        return retValue;
    }

    void static To(vm_view view, std::pair<K, V> const& value)
    {
        view.new_array(2);

        view.push_convert(0);
        converter<K>::To(view, value.first);
        view.set(-3);

        view.push_convert(1);
        converter<V>::To(view, value.second);
        view.set(-3);
    }
};

template <>
struct converter<std::filesystem::path> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(vm_view view, SQInteger& idx, std::filesystem::path& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.get_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, std::filesystem::path const& value)
    {
        view.push_string(value.string());
    }
};

////squirrel//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct ref_converter {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return T::IsType(view, idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        if (T::IsType(view, idx)) {
            value.acquire(view, idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view /* view */, T const& value)
    {
        value.push_self();
    }

    void static To(vm_view view, T& value)
    {
        if (!value.is_valid()) {
            if constexpr (requires { T::PushNew(view); }) {
                value = T::PushNew(view);
            }
        } else {
            value.push_self();
        }
    }
};

template <>
struct converter<table> : public ref_converter<table> { };
template <>
struct converter<stack_base> : public ref_converter<stack_base> { };

template <>
struct converter<clazz> : public ref_converter<clazz> { };

template <>
struct converter<instance> : public ref_converter<instance> { };

template <>
struct converter<array> : public ref_converter<array> { };

template <>
struct converter<generator> : public ref_converter<generator> { };

template <>
struct converter<thread> : public ref_converter<thread> { };

template <typename T>
struct converter<function<T>> : public ref_converter<function<T>> { };

////basic/////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <>
struct converter<char const*> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(vm_view view, SQInteger& idx, char const*& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.get_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, char const* value)
    {
        view.push_string(value);
    }
};

template <usize N>
struct converter<char const[N]> { // NOLINT
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    void static To(vm_view view, char const* value)
    {
        view.push_string(value);
    }
};

template <>
struct converter<string> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(vm_view view, SQInteger& idx, string& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.get_string(idx++);
            return true;
        }

        if (view.is_integer(idx)) {
            value = std::to_string(view.get_integer(idx++));
            return true;
        }

        if (view.is_number(idx)) {
            value = std::to_string(view.get_float(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, string const& value)
    {
        view.push_string(value);
    }
};

template <>
struct converter<string_view> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.get_type(idx) == type::String;
    }

    auto static From(vm_view view, SQInteger& idx, string_view& value) -> bool
    {
        if (view.is_string(idx)) {
            value = view.get_string(idx++);
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, string_view value)
    {
        view.push_string(value);
    }
};

template <>
struct converter<std::nullptr_t> {
    void static To(vm_view view, std::nullptr_t const&)
    {
        view.push_null();
    }
};

template <>
struct converter<bool> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.get_type(idx) == type::Boolean;
    }

    auto static From(vm_view view, SQInteger& idx, bool& value) -> bool
    {
        if (view.is_bool(idx)) {
            value = static_cast<bool>(view.get_bool(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, bool const& value)
    {
        view.push_bool(value);
    }
};

template <Enum T>
struct converter<T> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.is_integer(idx) || view.is_string(idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        if (view.is_integer(idx)) {
            value = static_cast<T>(view.get_integer(idx++));
            return true;
        }

        if (view.is_string(idx)) {
            value = tcob::detail::magic_enum_reduced::string_to_enum<T>(view.get_string(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, T const& value)
    {
        view.push_string(tcob::detail::magic_enum_reduced::enum_to_string(value));
    }
};

template <Integral T>
struct converter<T> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.is_integer(idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        if (view.is_integer(idx)) {
            value = static_cast<T>(view.get_integer(idx++));
            return true;
        }
        if (view.is_number(idx)) {
            f32 const val {view.get_float(idx++)};
            if (std::ceil(val) == val) {
                value = static_cast<T>(val);
                return true;
            }

            return false;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, T const& value)
    {
        view.push_integer(static_cast<SQInteger>(value));
    }
};

template <FloatingPoint T>
struct converter<T> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.is_number(idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        if (view.is_number(idx)) {
            value = static_cast<T>(view.get_float(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, T const& value)
    {
        view.push_float(static_cast<f32>(value));
    }
};

template <Pointer T>
struct converter<T> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.is_userdata(idx);
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        static string TypeName {typeid(std::remove_pointer_t<T>).name()};

        if (view.is_userdata(idx)) {
            void* typetag {nullptr};
            void* val {nullptr};
            view.get_userdata(idx++, &val, &typetag);
            if (typetag && val) {
                string const userDataType {static_cast<char*>(typetag)};

                // try typetag
                if (TypeName == userDataType) {
                    value = *static_cast<T*>(val);
                    return true;
                }

                // try metatable
                view.push_registrytable();
                table retTable {table::Acquire(view, -1)};
                view.pop(1);
                if (retTable.has(userDataType)) {
                    if (std::unordered_set<string> types; retTable[userDataType].try_get(types, "__types") && types.contains(TypeName)) {
                        value = *static_cast<T*>(val);
                        return true;
                    }
                }
            }
            return false;
        }

        if constexpr (std::is_same_v<T, char const*>) {
            if (view.is_string(idx)) {
                value = view.get_string(idx++);
                return true;
            }
        } else {
            idx++;
            return false;
        }
    }

    void static To(vm_view view, T const& value)
    {
        static string TypeName {typeid(std::remove_pointer_t<T>).name()};

        T* obj {static_cast<T*>(view.new_userdata(sizeof(T*)))};
        *obj = value;
        view.set_typetag(-1, TypeName.data());

        view.push_registrytable();
        table retTable {table::Acquire(view, -1)};
        if (retTable.has(TypeName)) {
            retTable.get<table>(TypeName)->push_self();
        } else {
            retTable[TypeName] = table::PushNew(view);
        }

        view.set_delegate(-3);
        view.pop(1);
    }
};

////tcob//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct converter<scripting::owned_ptr<T>> {
    void static To(vm_view view, scripting::owned_ptr<T> const& value)
    {
        converter<T*>::To(view, value.Pointer);
        if constexpr (std::is_destructible_v<T>) {
            view.set_releasehook(-1, &hook);
        }
    }

    auto static hook(void* ptr, SQInteger) -> SQInteger
    {
        T** obj {static_cast<T**>(ptr)};

        if (obj && *obj) {
            delete (*obj); // NOLINT
        }

        return 0;
    }
};

template <typename T>
struct converter<result<T>> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return converter<T>::IsType(view, idx);
    }

    auto static From(vm_view view, SQInteger& idx, result<T>& value) -> bool
    {
        T val;
        if (converter<T>::From(view, idx, val)) {
            value = result<T> {std::move(val)};
            return true;
        }

        value = result<T> {error_code::TypeMismatch};
        return false;
    }

    void static To(vm_view view, result<T> const& value)
    {
        converter<T>::To(view, value.value());
    }
};

template <Serializable<table> T>
struct converter<T> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        if (view.is_table(idx)) {
            table tab {table::Acquire(view, idx)};
            T     t {};
            return T::Deserialize(t, tab);
        }

        return false;
    }

    auto static From(vm_view view, SQInteger& idx, T& value) -> bool
    {
        table tab {table::Acquire(view, idx++)};
        return T::Deserialize(value, tab);
    }

    void static To(vm_view view, T const& value)
    {
        table tab {table::PushNew(view)};
        T::Serialize(value, tab);
    }
};

template <FloatingPoint T>
struct converter<degree<T>> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.is_number(idx);
    }

    auto static From(vm_view view, SQInteger& idx, degree<T>& value) -> bool
    {
        if (view.is_number(idx)) {
            value = static_cast<T>(view.get_float(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, degree<T> const& value)
    {
        view.push_float(static_cast<f32>(value.Value));
    }
};

template <FloatingPoint T>
struct converter<radian<T>> {
    auto static IsType(vm_view view, SQInteger idx) -> bool
    {
        return view.is_number(idx);
    }

    auto static From(vm_view view, SQInteger& idx, radian<T>& value) -> bool
    {
        if (view.is_number(idx)) {
            value = static_cast<T>(view.get_float(idx++));
            return true;
        }

        idx++;
        return false;
    }

    void static To(vm_view view, radian<T> const& value)
    {
        view.push_float(static_cast<f32>(value.Value));
    }
};

}
#endif
