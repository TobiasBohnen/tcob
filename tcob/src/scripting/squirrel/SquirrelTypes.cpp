// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/squirrel/SquirrelTypes.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <squirrel.h>

    #include <cassert>

    #include "tcob/core/io/FileStream.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"

namespace tcob::scripting::squirrel {

ref::ref()
    : _ref {std::make_shared<HSQOBJECT>()}
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
        _ref  = nullptr;
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

table::table(vm_view view, SQInteger idx)
{
    acquire(view, idx);
}

auto table::PushNew(vm_view view) -> table
{
    view.new_table();
    return table {view, -1};
}

auto table::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_table(idx);
}

////////////////////////////////////////////////////////////

auto detail::function_base::call_protected(SQInteger nargs, bool retValue) const -> error_code
{
    return get_view().call(nargs, retValue, true);
}

////////////////////////////////////////////////////////////

array::array() = default;

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

auto array::get_size() const -> SQInteger
{
    auto view {get_view()};
    auto guard {view.create_stack_guard()};

    push_self();
    return view.get_size(-1);
}

auto array::PushNew(vm_view view) -> array
{
    view.new_array(0);
    return array {view, -1};
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

auto thread::get_status() const -> vm_view::status
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

class_t::class_t() = default;

class_t::class_t(vm_view view, SQInteger idx)
{
    acquire(view, idx);
}

auto class_t::create_instance() const -> instance
{
    auto view {get_view()};
    auto guard {view.create_stack_guard()};

    push_self();
    view.create_instance(-1);
    return instance {view, -1};
}

auto class_t::PushNew(vm_view view) -> class_t
{
    view.new_class(false);
    return class_t {view, -1};
}

auto class_t::IsType(vm_view view, SQInteger idx) -> bool
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

auto instance::IsType(vm_view view, SQInteger idx) -> bool
{
    return view.is_instance(idx);
}

////////////////////////////////////////////////////////////

}

#endif
