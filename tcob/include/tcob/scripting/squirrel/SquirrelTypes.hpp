// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <future>
    #include <vector>

    #include "tcob/core/Proxy.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"

namespace tcob::scripting::squirrel {
////////////////////////////////////////////////////////////

class TCOB_API ref {
public:
    ref();
    ref(ref const& other) noexcept;
    auto operator=(ref const& other) noexcept -> ref&;
    ref(ref&& other) noexcept;
    auto operator=(ref&& other) noexcept -> ref&;
    virtual ~ref();

    auto is_valid() const -> bool;

    void acquire(vm_view view, SQInteger idx);
    void release();

    void push_self() const;

    friend auto operator==(ref const& left, ref const& right) -> bool;

protected:
    auto get_view() const -> vm_view;

private:
    std::unique_ptr<HSQOBJECT> _ref;

    vm_view _view {nullptr};
};

////////////////////////////////////////////////////////////

class TCOB_API table : public ref {
public:
    table();
    explicit table(vm_view view);

    template <typename Key>
    auto operator[](Key key) -> proxy<table, Key>;

    template <typename Key>
    auto operator[](Key key) const -> proxy<table const, Key>;

    auto get_delegate() const -> table;
    void set_delegate(table const& mt) const;

    template <ConvertibleFrom T>
    auto get(auto&&... keys) const -> result<T>;

    template <ConvertibleFrom T>
    auto try_get(T& value, auto&& key) const -> bool;

    void set(auto&&... keysOrValue) const;

    template <typename T>
    auto is(auto&&... keys) const -> bool;

    auto has(auto&&... keys) const -> bool;

    template <typename T>
    auto get_keys() const -> std::vector<T>;

    auto static PushNew(vm_view view) -> table;
    auto static Acquire(vm_view view, SQInteger idx) -> table;
    auto static IsType(vm_view view, SQInteger idx) -> bool;

private:
    table(vm_view view, SQInteger idx);

    template <typename T>
    auto get(vm_view view, auto&& key, auto&&... keys) const -> result<T>;

    void set(vm_view view, auto&& key, auto&&... keys) const;

    template <typename T>
    auto is(vm_view view, auto&& key, auto&&... keys) const -> bool;

    auto has(vm_view view, auto&& key, auto&&... keys) const -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API stack_base : public table {
};

////////////////////////////////////////////////////////////

class TCOB_API array : public ref {
public:
    array();
    explicit array(vm_view view);

    auto operator[](SQInteger index) -> proxy<array, SQInteger>;
    auto operator[](SQInteger index) const -> proxy<array const, SQInteger>;

    auto get_size() const -> SQInteger;

    template <ConvertibleFrom T>
    auto get(SQInteger index) const -> result<T>;

    void set(SQInteger index, auto&& value);

    template <ConvertibleFrom T>
    auto is(SQInteger index) const -> bool;

    template <ConvertibleTo T>
    void add(T const& addValue);

    auto static PushNew(vm_view view) -> array;
    auto static Acquire(vm_view view, SQInteger idx) -> array;
    auto static IsType(vm_view view, SQInteger idx) -> bool;

private:
    array(vm_view view, SQInteger idx);
};

////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API type_ref : public ref {
    public:
        template <ConvertibleFrom T>
        auto get(auto&& key) const -> result<T>;

        template <ConvertibleFrom T>
        auto try_get(T& value, auto&& key) const -> bool;

        template <typename T>
        auto is(auto&& key) const -> bool;

        auto has(auto&& key) const -> bool;
    };
}

////////////////////////////////////////////////////////////

class TCOB_API instance : public detail::type_ref {
public:
    instance();
    instance(vm_view view, SQInteger idx);

    template <typename Key>
    auto operator[](Key key) -> proxy<instance, Key>;

    template <typename Key>
    auto operator[](Key key) const -> proxy<instance const, Key>;

    void set(auto&& key, auto&& value) const;

    auto static IsType(vm_view view, SQInteger idx) -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API clazz : public detail::type_ref {
public:
    clazz();
    explicit clazz(vm_view view);

    template <typename Key>
    auto operator[](Key key) -> proxy<clazz, Key>;

    template <typename Key>
    auto operator[](Key key) const -> proxy<clazz const, Key>;

    void set(auto&& key, auto&& value) const;

    auto create_instance() const -> instance;

    auto static PushNew(vm_view view) -> clazz;
    auto static Acquire(vm_view view, SQInteger idx) -> clazz;
    auto static IsType(vm_view view, SQInteger idx) -> bool;

private:
    clazz(vm_view view, SQInteger idx);
};

////////////////////////////////////////////////////////////

namespace detail {
    ////////////////////////////////////////////////////////////

    class TCOB_API function_base : public ref {
    protected:
        auto call_protected(SQInteger nargs, bool retValue) const -> error_code;
    };
}

////////////////////////////////////////////////////////////

template <typename R>
class function final : public detail::function_base {
public:
    using return_type = R;

    auto operator()(auto&&... params) const -> return_type;

    auto call(auto&&... params) const -> result<return_type>;
    auto call_async(auto&&... params) const -> std::future<result<return_type>>;

    auto static IsType(vm_view view, SQInteger idx) -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API generator final : public ref {
public:
    template <typename R = void>
    auto resume() const -> result<R>;

    auto static IsType(vm_view view, SQInteger idx) -> bool;
};

////////////////////////////////////////////////////////////

class TCOB_API thread final : public ref {
public:
    template <typename R>
    auto call(auto&&... params) const -> result<R>;

    auto suspend() const -> bool;

    template <typename R = void>
    auto wake_up() const -> result<R>;

    template <typename R = void>
    auto wake_up(auto&& arg) const -> result<R>;

    auto get_status() const -> vm_view::status;

    auto static IsType(vm_view view, SQInteger idx) -> bool;

private:
    auto get_thread() const -> vm_view;
};

}

    #include "SquirrelTypes.inl"

#endif
