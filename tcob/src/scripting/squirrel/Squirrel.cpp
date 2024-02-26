// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/squirrel/Squirrel.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include "tcob/core/FlatMap.hpp"

    #include <squirrel.h>

    #include <sqstdaux.h>
    #include <sqstdblob.h>
    #include <sqstdio.h>
    #include <sqstdmath.h>
    #include <sqstdstring.h>
    #include <sqstdsystem.h>

namespace tcob::scripting::squirrel {

////////////////////////////////////////////////////////////

stack_guard::stack_guard(HSQUIRRELVM vm)
    : _vm {vm}
    , _oldTop {sq_gettop(vm)}
{
}

stack_guard::~stack_guard()
{
    SQInteger const n {sq_gettop(_vm) - _oldTop};
    if (n > 0) {
        sq_pop(_vm, n);
    }
}

auto stack_guard::get_top() const -> SQInteger
{
    return _oldTop;
}

////////////////////////////////////////////////////////////

vm_view::vm_view(HSQUIRRELVM vm)
    : _vm {vm}
{
}

auto vm_view::create_stack_guard() const -> stack_guard
{
    return stack_guard {_vm};
}

auto vm_view::is_array(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isarray(o);
}

auto vm_view::is_bool(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isbool(o);
}

auto vm_view::is_function(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isfunction(o);
}

auto vm_view::is_closure(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isclosure(o);
}

auto vm_view::is_nativeclosure(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isnativeclosure(o);
}

auto vm_view::is_integer(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isinteger(o);
}

auto vm_view::is_number(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isnumeric(o);
}

auto vm_view::is_string(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isstring(o);
}

auto vm_view::is_table(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_istable(o);
}

auto vm_view::is_null(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isnull(o);
}

auto vm_view::is_generator(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isgenerator(o);
}

auto vm_view::is_thread(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isthread(o);
}

auto vm_view::is_userpointer(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isuserpointer(o);
}

auto vm_view::is_userdata(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isuserdata(o);
}

auto vm_view::is_class(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isclass(o);
}

auto vm_view::is_instance(SQInteger idx) const -> bool
{
    HSQOBJECT o;
    sq_resetobject(&o);
    sq_getstackobj(_vm, idx, &o);
    return sq_isinstance(o);
}

auto vm_view::get_bool(SQInteger idx) const -> bool
{
    SQBool retValue {false};
    sq_getbool(_vm, idx, &retValue);
    return retValue != 0;
}

auto vm_view::get_integer(SQInteger idx) const -> SQInteger
{
    SQInteger retValue {0};
    sq_getinteger(_vm, idx, &retValue);
    return static_cast<SQInteger>(retValue);
}

auto vm_view::get_float(SQInteger idx) const -> f32
{
    SQFloat retValue {0};
    sq_getfloat(_vm, idx, &retValue);
    return static_cast<f32>(retValue);
}

auto vm_view::get_string(SQInteger idx) const -> char const*
{
    char const* retValue {nullptr};
    sq_getstring(_vm, idx, &retValue);
    return retValue;
}

void vm_view::get_stackobj(SQInteger idx, HSQOBJECT* o)
{
    sq_getstackobj(_vm, idx, o);
}

auto vm_view::get_thread(SQInteger idx) const -> vm_view
{
    HSQUIRRELVM thread {nullptr};
    sq_getthread(_vm, idx, &thread);
    return vm_view {thread};
}

auto vm_view::get_userpointer(SQInteger idx, void** p) const -> bool
{
    return sq_getuserpointer(_vm, idx, p) == SQ_OK;
}

auto vm_view::get_userdata(SQInteger idx, void** p, void** typetag) const -> bool
{
    return sq_getuserdata(_vm, idx, p, typetag) == SQ_OK;
}

auto vm_view::get_class(SQInteger idx) const -> bool
{
    return sq_getclass(_vm, idx) == SQ_OK;
}

auto vm_view::new_slot(SQInteger idx, bool isStatic) const -> bool
{
    return sq_newslot(_vm, idx, isStatic) == SQ_OK;
}

auto vm_view::set(SQInteger idx) const -> bool
{
    return sq_set(_vm, idx) == SQ_OK;
}

auto vm_view::get(SQInteger idx) const -> bool
{
    return sq_get(_vm, idx) == SQ_OK;
}

auto vm_view::get_type(SQInteger idx) const -> type
{
    static flat_map<SQObjectType, type> const typeMap {
        {SQObjectType::OT_NULL, type::Null},
        {SQObjectType::OT_INTEGER, type::Integer},
        {SQObjectType::OT_FLOAT, type::Float},
        {SQObjectType::OT_BOOL, type::Boolean},
        {SQObjectType::OT_STRING, type::String},
        {SQObjectType::OT_TABLE, type::Table},
        {SQObjectType::OT_ARRAY, type::Array},
        {SQObjectType::OT_USERDATA, type::Userdata},
        {SQObjectType::OT_CLOSURE, type::Closure},
        {SQObjectType::OT_NATIVECLOSURE, type::NativeClosure},
        {SQObjectType::OT_GENERATOR, type::Generator},
        {SQObjectType::OT_USERPOINTER, type::UserPointer},
        {SQObjectType::OT_THREAD, type::Thread},
        {SQObjectType::OT_CLASS, type::Class},
        {SQObjectType::OT_INSTANCE, type::Instance},
        {SQObjectType::OT_WEAKREF, type::WeakReference}};

    return typeMap.at(sq_gettype(_vm, idx));
}

auto vm_view::get_size(SQInteger idx) const -> SQInteger
{
    return sq_getsize(_vm, idx);
}

auto vm_view::get_top() const -> SQInteger
{
    return sq_gettop(_vm);
}

auto vm_view::get_functioninfo(SQInteger level) const -> function_info
{
    SQFunctionInfo fi;
    if (sq_getfunctioninfo(_vm, level, &fi) == SQ_OK) {
        return {.FuncID = fi.funcid,
                .Line   = fi.line,
                .Name   = fi.name,
                .Source = fi.source};
    }

    return {};
}

auto vm_view::next(SQInteger idx) const -> bool
{
    return sq_next(_vm, idx) == SQ_OK;
}

void vm_view::push(SQInteger idx) const
{
    sq_push(_vm, idx);
}

void vm_view::push_bool(bool val) const
{
    sq_pushbool(_vm, val);
}

void vm_view::push_integer(SQInteger val) const
{
    sq_pushinteger(_vm, val);
}

void vm_view::push_float(f32 val) const
{
    sq_pushfloat(_vm, val);
}

void vm_view::push_null() const
{
    sq_pushnull(_vm);
}

void vm_view::push_object(HSQOBJECT* po) const
{
    sq_pushobject(_vm, *po);
}

void vm_view::push_string(string_view val) const
{
    sq_pushstring(_vm, val.data(), std::ssize(val));
}

void vm_view::push_roottable() const
{
    sq_pushroottable(_vm);
}

void vm_view::push_userpointer(void* ptr) const
{
    sq_pushuserpointer(_vm, ptr);
}

void vm_view::push_registrytable() const
{
    sq_pushregistrytable(_vm);
}

auto vm_view::array_append(SQInteger idx) const -> bool
{
    return sq_arrayappend(_vm, idx) == SQ_OK;
}

auto vm_view::array_insert(SQInteger idx, SQInteger dest) const -> bool
{
    return sq_arrayinsert(_vm, idx, dest) == SQ_OK;
}

void vm_view::pop(SQInteger count) const
{
    sq_pop(_vm, count);
}

void vm_view::poptop() const
{
    sq_poptop(_vm);
}

void vm_view::remove(SQInteger idx) const
{
    sq_remove(_vm, idx);
}

void vm_view::new_array(SQInteger size) const
{
    sq_newarray(_vm, size);
}

void vm_view::new_table() const
{
    sq_newtable(_vm);
}

void vm_view::new_table(SQInteger size) const
{
    sq_newtableex(_vm, size);
}

void vm_view::new_closure(SQFUNCTION func, SQUnsignedInteger freeVars) const
{
    sq_newclosure(_vm, func, freeVars);
}

auto vm_view::new_thread(SQInteger initialstacksize) const -> vm_view
{
    return vm_view {sq_newthread(_vm, initialstacksize)};
}

auto vm_view::new_userdata(SQUnsignedInteger size) const -> void*
{
    return sq_newuserdata(_vm, size);
}

auto vm_view::new_class(bool hasbase) const -> bool
{
    return sq_newclass(_vm, hasbase) == SQ_OK;
}

auto vm_view::create_instance(SQInteger idx) const -> bool
{
    return sq_createinstance(_vm, idx) == SQ_OK;
}

auto vm_view::set_typetag(SQInteger idx, void* typetag) const -> bool
{
    return sq_settypetag(_vm, idx, typetag) == SQ_OK;
}

auto vm_view::set_delegate(SQInteger idx) const -> bool
{
    return sq_setdelegate(_vm, idx) == SQ_OK;
}

auto vm_view::get_delegate(SQInteger idx) const -> bool
{
    return sq_getdelegate(_vm, idx) == SQ_OK;
}

auto vm_view::raw_get(SQInteger idx) const -> bool
{
    return sq_rawget(_vm, idx) == SQTrue;
}

auto vm_view::raw_set(SQInteger idx) const -> bool
{
    return sq_rawset(_vm, idx) == SQTrue;
}

void vm_view::add_ref(HSQOBJECT* po) const
{
    sq_addref(_vm, po);
}

auto vm_view::release(HSQOBJECT* po) const -> bool
{
    return sq_release(_vm, po) == SQTrue;
}

auto vm_view::resume(bool retVal) const -> bool
{
    return sq_resume(_vm, retVal, true) == SQ_OK;
}

auto vm_view::suspend_vm() const -> bool
{
    return sq_suspendvm(_vm) == SQ_OK;
}

auto vm_view::wakeup_vm(bool resumedret, bool retval) const -> bool
{
    return sq_wakeupvm(_vm, resumedret, retval, true, false) == SQ_OK;
}

auto vm_view::get_vm_state() const -> status
{
    auto const state {sq_getvmstate(_vm)};
    switch (state) {
    case SQ_VMSTATE_IDLE:
        return status::Idle;
    case SQ_VMSTATE_RUNNING:
        return status::Running;
    case SQ_VMSTATE_SUSPENDED:
        return status::Suspended;
    }

    return status::Suspended;
}

void vm_view::reset_object(HSQOBJECT* po) const
{
    sq_resetobject(po);
}

void vm_view::throw_error(string const& message) const
{
    sq_throwerror(_vm, message.c_str());
}

auto vm_view::call(SQInteger params, bool retVal, bool raiseError) const -> error_code
{
    return sq_call(_vm, params, retVal, raiseError) == SQ_OK ? error_code::Ok : error_code::Error;
}

auto vm_view::compile_buffer(string_view script, string const& name) const -> error_code
{
    return sq_compilebuffer(_vm, script.data(), std::ssize(script), name.c_str(), true) == SQ_OK ? error_code::Ok : error_code::Error;
}

void vm_view::set_print_func(SQPRINTFUNCTION printfunc, SQPRINTFUNCTION errfunc) const
{
    sq_setprintfunc(_vm, printfunc, errfunc);
}

void vm_view::set_compiler_errorhandler(SQCOMPILERERROR func) const
{
    sq_setcompilererrorhandler(_vm, func);
}

void vm_view::set_errorhandler() const
{
    sq_seterrorhandler(_vm);
}

void vm_view::set_releasehook(SQInteger idx, SQRELEASEHOOK hook) const
{
    sq_setreleasehook(_vm, idx, hook);
}

auto vm_view::cmp() const -> SQInteger
{
    return sq_cmp(_vm);
}

void vm_view::register_iolib() const
{
    sqstd_register_iolib(_vm);
}

void vm_view::register_bloblib() const
{
    sqstd_register_bloblib(_vm);
}

void vm_view::register_mathlib() const
{
    sqstd_register_mathlib(_vm);
}

void vm_view::register_systemlib() const
{
    sqstd_register_systemlib(_vm);
}

void vm_view::register_stringlib() const
{
    sqstd_register_stringlib(_vm);
}

auto vm_view::NewVM() -> HSQUIRRELVM
{
    return sq_open(1024);
}

void vm_view::close() const
{
    sq_close(_vm);
}

auto vm_view::is_valid() const -> bool
{
    return _vm != nullptr;
}

auto vm_view::get_stack_types() const -> std::vector<type>
{
    std::vector<type> retValue {};
    SQInteger const   top {get_top()};

    for (SQInteger i {1}; i <= top; ++i) {
        retValue.push_back(get_type(i));
    }

    return retValue;
}

}

#endif
