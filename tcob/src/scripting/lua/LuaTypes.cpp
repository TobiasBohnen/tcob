// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/lua/LuaTypes.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_LUA)

    #include <algorithm>
    #include <cassert>
    #include <optional>
    #include <unordered_set>
    #include <utility>
    #include <variant>
    #include <vector>

    #include "tcob/core/io/Stream.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/lua/Lua.hpp"

namespace tcob::scripting::lua {

ref::ref() = default;

ref::ref(ref const& other) noexcept
    : ref {}
{
    *this = other;
}

auto ref::operator=(ref const& other) noexcept -> ref&
{
    if (this != &other) {
        if (other._view.is_valid()) {
            other.push_self();
            acquire(other._view, -1);
            if (_ref != NOREF) {
                _view.pop(1); // pop extra copy
            }
        }
    }

    return *this;
}

ref::ref(ref&& other) noexcept
    : _view {std::exchange(other._view, state_view {nullptr})}
    , _ref {std::exchange(other._ref, NOREF)}
{
}

auto ref::operator=(ref&& other) noexcept -> ref&
{
    std::swap(_view, other._view);
    std::swap(_ref, other._ref);
    return *this;
}

ref::~ref()
{
    release();
}

void ref::acquire(state_view view, i32 idx)
{
    release();
    _view = view;

    if (_view.is_valid()) {
        _view.push_value(idx); // push copy of ref to top
        _ref = _view.ref(REGISTRYINDEX);
    }
}

void ref::release()
{
    if (is_valid()) {
        _view.unref(REGISTRYINDEX, _ref);
        _ref  = NOREF;
        _view = state_view {nullptr};
    }
}

void ref::push_self() const
{
    assert(is_valid());
    _view.raw_get(REGISTRYINDEX, _ref);
}

auto ref::get_view() const -> state_view
{
    return _view;
}

ref::operator bool() const
{
    return is_valid();
}

auto ref::is_valid() const -> bool
{
    return _ref != NOREF && _view.is_valid();
}

auto operator==(ref const& left, ref const& right) -> bool
{
    auto const guard {left._view.create_stack_guard()};
    left.push_self();
    right.push_self();
    return left._view.raw_equal(-1, -2);
}

////////////////////////////////////////////////////////////

table::table() = default;

table::table(state_view view)
{
    auto const guard {view.create_stack_guard()};
    view.new_table();
    acquire(view, -1);
}

table::table(state_view view, i32 idx)
{
    acquire(view, idx);
}

auto table::raw_length() const -> u64
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    return view.raw_len(-1);
}

auto table::create_or_get_metatable() const -> table
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    table retValue {};

    push_self();
    if (!view.get_metatable(-1)) {
        view.new_table();
        retValue.acquire(view, -1);
        view.set_metatable(-2);
    }
    if (!view.get_metatable(-1)) {
        retValue.acquire(view, -1);
    }

    return retValue;
}

void table::set_metatable(table const& mt) const
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    mt.push_self();
    view.set_metatable(-2);
}

auto table::Create(state_view view) -> table
{
    return table {view};
}

auto table::PushNew(state_view view) -> table
{
    view.new_table();
    return table {view, -1};
}

auto table::Acquire(state_view view, i32 idx) -> table
{
    return table {view, idx};
}

void table::dump(io::ostream& stream) const
{
    auto const guard {get_view().create_stack_guard()};
    push_self();
    write_to_stream(stream, 0);
    stream << '\n';
}

void table::write_to_stream(io::ostream& stream, usize indent) const
{
    auto const view {get_view()};

    string ind1(indent, ' ');
    string ind2(indent + 2, ' ');

    stream << "{" << '\n';

    std::vector<std::variant<i64, string>> keys;
    view.push_nil();
    while (view.next(-2)) {
        view.push_value(-2);
        if (view.is_integer(-1)) {
            keys.emplace_back(view.to_integer(-1));
        } else if (view.is_string(-1)) {
            keys.emplace_back(view.to_string(-1));
        }

        view.pop(2);
    }
    std::ranges::sort(keys);

    for (auto const& key : keys) {
        // push key
        if (auto const* arg0 {std::get_if<string>(&key)}) {
            view.push_string(*arg0);
        } else if (auto const* arg1 {std::get_if<i64>(&key)}) {
            view.push_integer(*arg1);
        }

        view.raw_get(-2); // get value

        type type {view.get_type(-1)};
        switch (type) {
        case type::Nil:
        case type::LightUserdata:
        case type::Function:
        case type::Userdata:
        case type::Thread:
            view.pop(1);
            continue;
        default:
            if (std::holds_alternative<string>(key)) {
                stream << ind2 << std::get<string>(key) << " = ";
            } else if (std::holds_alternative<i64>(key)) {
                stream << ind2;
            }
            break;
        }

        string val;
        switch (type) {
        case type::Boolean:
            val = view.to_bool(-1) ? "true" : "false";
            break;
        case type::Number:
            val = view.to_string(-1);
            break;
        case type::String:
            val = "\"" + string(view.to_string(-1)) + "\"";
            break;
        case type::Table:
            view.push_value(-1);
            write_to_stream(stream, indent + 2);
            break;
        default:
            break;
        }
        stream << val << "," << '\n';

        view.pop(1);
    }

    view.pop(1);
    stream << ind1 << "}";
}

////////////////////////////////////////////////////////////

namespace detail {
    function_base::function_base(state_view view, i32 idx)
    {
        acquire(view, idx);
    }

    static auto writer(lua_State*, void const* p, usize sz, void* ud) -> i32
    {
        auto* stream {static_cast<io::ostream*>(ud)};
        stream->write<byte>({static_cast<byte const*>(p), sz});
        return 0;
    }

    void function_base::dump(io::ostream& stream) const
    {
        auto const view {get_view()};
        push_self();
        view.dump(&writer, &stream, true);
        view.pop(1);
    }

    void function_base::call(i32 nargs) const
    {
        get_view().call(nargs);
    }

    auto function_base::pcall(i32 nargs) const -> std::optional<error_code>
    {
        return get_view().pcall(nargs);
    }

    auto function_base::get_upvalues() const -> std::unordered_set<string>
    {
        std::unordered_set<string> retValue;

        auto const view {get_view()};
        auto       guard {view.create_stack_guard()};

        push_self();

        for (i32 i {1};; ++i) {
            auto const* upname {view.get_upvalue(-1, i)};
            if (upname == nullptr) { break; }

            view.pop(1);

            retValue.insert(upname);
        }

        return retValue;
    }

    auto function_base::set_upvalue(string const& name, ref const& value) -> bool
    {
        auto const view {get_view()};
        auto       guard {view.create_stack_guard()};

        push_self();

        for (i32 i {1};; ++i) {
            auto const* upname {view.get_upvalue(-1, i)};
            if (upname == nullptr) { break; }

            view.pop(1);

            if (upname == name) {
                value.push_self();
                return view.set_upvalue(-2, i);
            }
        }

        return false;
    }

    auto function_base::set_environment([[maybe_unused]] table const& env) -> bool
    {
        return set_upvalue("_ENV", env);
    }

}

////////////////////////////////////////////////////////////

auto coroutine::close() -> coroutine_status
{
    _status = get_thread().close_thread() ? coroutine_status::Dead : coroutine_status::Error;
    return _status;
}

auto coroutine::status() const -> coroutine_status
{
    return is_valid() ? _status : coroutine_status::Error;
}

auto coroutine::get_thread() const -> state_view
{
    push_self();
    auto const view {get_view()};
    auto const retValue {view.to_thread(-1)};
    view.pop(1);
    return retValue;
}

}

#endif
