// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <unordered_set>
    #include <vector>

    #include "tcob/core/Proxy.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/lua/Lua.hpp"

namespace tcob::scripting::lua {
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

    void acquire(state_view view, i32 idx);
    void release();

    void push_self() const;

    friend auto operator==(ref const& left, ref const& right) -> bool;

protected:
    auto get_view() const -> state_view;

private:
    state_view _view {nullptr};
    i32        _ref {-2};
};

////////////////////////////////////////////////////////////

class TCOB_API table : public ref {
public:
    table();
    explicit table(state_view view);

    template <typename Key>
    auto operator[](Key key) -> proxy<table, Key>;
    template <typename Key>
    auto operator[](Key key) const -> proxy<table const, Key>;

    auto create_or_get_metatable() const -> table;
    void set_metatable(table const& mt) const;

    auto raw_length() const -> u64;

    template <typename... Args, typename T>
    auto try_make(T& value, auto&&... keys) const -> bool;

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

    void dump(io::ostream& stream) const;

    auto static PushNew(state_view view) -> table;
    auto static Acquire(state_view view, i32 idx) -> table;

private:
    table(state_view view, i32 idx);

    void write_to_stream(io::ostream& stream, usize indent) const;

    template <typename T>
    auto get(state_view view, auto&& key, auto&&... keys) const -> result<T>;

    void set(state_view view, auto&& key, auto&&... keys) const;

    template <typename T>
    auto is(state_view view, auto&& key, auto&&... keys) const -> bool;

    auto has(state_view view, auto&& key, auto&&... keys) const -> bool;
};

////////////////////////////////////////////////////////////

namespace detail {
    ////////////////////////////////////////////////////////////

    class TCOB_API function_base : public ref {
    public:
        function_base() = default;

        void dump(io::ostream& stream) const;

        auto get_upvalues() const -> std::unordered_set<string>; // TODO: set_get_
        auto set_upvalue(string const& name, ref const& value) -> bool;

        auto set_environment(table const& env) -> bool;

    protected:
        function_base(state_view view, i32 idx);

        auto upcall(i32 nargs) const -> error_code;
        auto pcall(i32 nargs) const -> error_code;
    };
}

////////////////////////////////////////////////////////////

template <typename R>
class function final : public detail::function_base {
public:
    using return_type = R;

    function() = default;

    auto operator()(auto&&... params) const -> return_type;
    auto protected_call(auto&&... params) const -> result<return_type>;
    auto unprotected_call(auto&&... params) const -> result<return_type>;

    auto static Acquire(state_view view, i32 idx) -> function<R>;

protected:
    using detail::function_base::function_base;
};

////////////////////////////////////////////////////////////

class TCOB_API coroutine final : public ref {
public:
    template <typename R = void>
    auto resume(auto&&... params) -> result<R>;

    void push(auto&&... values) const;

    auto close() -> coroutine_status;

    auto status() const -> coroutine_status;

private:
    auto get_thread() const -> state_view;

    coroutine_status _status {coroutine_status::Ok};
};

}

    #include "LuaTypes.inl"

#endif
