// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/squirrel/SquirrelTypes.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <squirrel.h>

    #include <cassert>
    #include <memory>
    #include <optional>
    #include <tuple>
    #include <utility>

    #include "tcob/core/Proxy.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"

namespace tcob::scripting::squirrel {

ref::ref()
    : _ref {std::make_unique<HSQOBJECT>()}
{
}

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
            if (_ref != nullptr) {
                _view.pop(1); // pop extra copy
            }
        }
    }

    return *this;
}

ref::ref(ref&& other) noexcept
    : _ref {std::exchange(other._ref, {nullptr})}
    , _view {std::exchange(other._view, vm_view {nullptr})}
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

void ref::acquire(vm_view view, SQInteger idx)
{
    release();
    _view = view;

    if (_view.is_valid()) {
        _view.push(idx); // push copy of ref to top
        _view.reset_object(_ref.get());
        _view.get_stackobj(idx, _ref.get());
        _view.add_ref(_ref.get());
        _view.pop(1);
    }
}

void ref::release()
{
    if (is_valid()) {
        _view.release(_ref.get());
        _ref  = std::make_unique<HSQOBJECT>();
        _view = vm_view {nullptr};
    }
}

void ref::push_self() const
{
    assert(is_valid());
    _view.push_object(_ref.get());
}

auto ref::get_view() const -> vm_view
{
    return _view;
}

auto ref::is_valid() const -> bool
{
    return _ref != nullptr && _view.is_valid();
}

auto operator==(ref const& left, ref const& right) -> bool
{
    auto const guard {left._view.create_stack_guard()};
    right.push_self();
    left.push_self();
    return left._view.cmp() == 0;
}

////////////////////////////////////////////////////////////

table::table() = default;

table::table(vm_view view)
{
    auto guard {view.create_stack_guard()};
    view.new_table();
    acquire(view, -1);
}

table::table(vm_view view, SQInteger idx)
{
    acquire(view, idx);
}

auto table::get_delegate() const -> table
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    table retValue {};

    push_self();
    if (view.get_delegate(-1) != 0) {
        retValue.acquire(view, -1);
    }

    return retValue;
}

void table::set_delegate(table const& mt) const
{
    auto const view {get_view()};
    auto const guard {view.create_stack_guard()};

    push_self();
    mt.push_self();
    view.set_delegate(-2);
}

auto table::Create(vm_view view) -> table
{
    return table {view};
}

auto table::PushNew(vm_view view) -> table
{
    view.new_table();
    return table {view, -1};
}

auto table::Acquire(vm_view view, SQInteger idx) -> table
{
    return table {view, idx};
}

auto table::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_table(idx);
}

////////////////////////////////////////////////////////////

detail::function_base::function_base(vm_view view, SQInteger idx)
{
    acquire(view, idx);
}

auto detail::function_base::upcall(SQInteger nargs, bool retValue) const -> std::optional<error_code>
{
    return get_view().call(nargs, retValue, true);
}

////////////////////////////////////////////////////////////

array::array() = default;

array::array(vm_view view)
{
    auto guard {view.create_stack_guard()};
    view.new_array(0);
    acquire(view, -1);
}

array::array(vm_view view, SQInteger idx)
{
    acquire(view, idx);
}

auto array::operator[](SQInteger index) -> proxy<array, SQInteger>
{
    return proxy<array, SQInteger> {*this, std::tuple {index}};
}

auto array::operator[](SQInteger index) const -> proxy<array const, SQInteger>
{
    return proxy<array const, SQInteger> {*this, std::tuple {index}};
}

auto array::size() const -> SQInteger
{
    auto view {get_view()};
    auto guard {view.create_stack_guard()};

    push_self();
    return view.get_size(-1);
}

auto array::Create(vm_view view) -> array
{
    return array {view};
}

auto array::PushNew(vm_view view) -> array
{
    view.new_array(0);
    return array {view, -1};
}

auto array::Acquire(vm_view view, SQInteger idx) -> array
{
    return array {view, idx};
}

auto array::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_array(idx);
}

////////////////////////////////////////////////////////////

auto thread::suspend() const -> bool
{
    return get_thread().suspend_vm();
}

auto thread::status() const -> vm_view::status
{
    return get_thread().get_vm_state();
}

auto thread::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_thread(idx);
}

auto thread::get_thread() const -> vm_view
{
    auto const view {get_view()};
    push_self();
    auto const thread {view.get_thread(-1)};
    view.pop(-1);
    return thread;
}

////////////////////////////////////////////////////////////

clazz::clazz() = default;

clazz::clazz(vm_view view)
{
    auto guard {view.create_stack_guard()};
    view.new_class(false);
    acquire(view, -1);
}

clazz::clazz(vm_view view, SQInteger idx)
{
    acquire(view, idx);
}

auto clazz::create_instance() const -> instance
{
    auto view {get_view()};
    auto guard {view.create_stack_guard()};

    push_self();
    view.create_instance(-1);
    return instance::Acquire(view, -1);
}

auto clazz::Create(vm_view view) -> clazz
{
    return clazz {view};
}

auto clazz::PushNew(vm_view view) -> clazz
{
    view.new_class(false);
    return clazz {view, -1};
}

auto clazz::Acquire(vm_view view, SQInteger idx) -> clazz
{
    return clazz {view, idx};
}

auto clazz::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_class(idx);
}

////////////////////////////////////////////////////////////

auto generator::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_generator(idx);
}

////////////////////////////////////////////////////////////

instance::instance() = default;

instance::instance(vm_view view, SQInteger idx)
{
    acquire(view, idx);
}

auto instance::Acquire(vm_view view, SQInteger idx) -> instance
{
    return instance {view, idx};
}

auto instance::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_instance(idx);
}

////////////////////////////////////////////////////////////
}

#endif
