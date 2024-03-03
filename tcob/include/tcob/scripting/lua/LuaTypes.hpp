// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <future>
    #include <unordered_set>
    #include <vector>

    #include "tcob/core/Proxy.hpp"
    #include "tcob/core/io/Stream.hpp"
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

    auto get_metatable() const -> table;
    void set_metatable(table const& mt) const;

    auto get_raw_length() const -> u64;

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

    void dump(ostream& stream) const;

    auto static PushNew(state_view view) -> table;
    auto static Acquire(state_view view, i32 idx) -> table;

private:
    table(state_view view, i32 idx);

    void write_to_stream(ostream& stream, usize indent) const;

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
        void dump(ostream& stream) const;

        auto get_upvalues() const -> std::unordered_set<string>;
        auto set_upvalue(string const& name, ref const& value) -> bool;

    protected:
        auto call_protected(i32 nargs) const -> error_code;
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
};

////////////////////////////////////////////////////////////

class TCOB_API coroutine final : public ref {
public:
    template <typename R = void>
    auto resume(auto&&... params) -> result<R>;

    void push(auto&&... values) const;

    auto close() -> coroutine_status;

    auto get_status() const -> coroutine_status;

private:
    auto get_thread() const -> state_view;

    coroutine_status _status {coroutine_status::Ok};
};

}

#endif

#include "LuaTypes.inl"
