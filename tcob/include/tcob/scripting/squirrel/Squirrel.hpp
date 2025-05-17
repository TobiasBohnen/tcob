// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"
#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <optional>
    #include <type_traits>
    #include <vector>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/scripting/Scripting.hpp"

    #if (defined(_WIN64) || defined(_LP64))
        #if defined(_MSC_VER)
using SQInteger         = tcob::i64;
using SQUnsignedInteger = tcob::u64;
        #else
using SQInteger         = long long;
using SQUnsignedInteger = unsigned long long;
        #endif
    #else
using SQInteger         = int;
using SQUnsignedInteger = unsigned int;
    #endif

struct SQVM;
using HSQUIRRELVM = SQVM*;
struct tagSQObject;
using HSQOBJECT = tagSQObject;
using SQChar    = char;

using SQFUNCTION      = SQInteger (*)(HSQUIRRELVM);
using SQCOMPILERERROR = void (*)(HSQUIRRELVM, char const* /*desc*/, char const* /*source*/, SQInteger /*line*/, SQInteger /*column*/);
using SQPRINTFUNCTION = void (*)(HSQUIRRELVM, char const*, ...);
using SQRELEASEHOOK   = SQInteger (*)(void*, SQInteger);
using SQDEBUGHOOK     = void (*)(HSQUIRRELVM /*v*/, SQInteger /*type*/, SQChar const* /*sourcename*/, SQInteger /*line*/, SQChar const* /*funcname*/);

namespace tcob::scripting::squirrel {
////////////////////////////////////////////////////////////

template <typename T>
struct converter;

class vm_view;

template <typename T>
concept ConvertibleTo =
    requires(T& t, vm_view state) {
        { converter<T>::To(state, t) };
    } || requires(T& t, vm_view state) {
        { converter<std::remove_cvref_t<T>>::To(state, t) };
    };

template <typename T>
concept ConvertibleFrom =
    requires(T& t, SQInteger& idx, vm_view state) {
        { converter<T>::From(state, idx, t) };
    } || requires(T& t, SQInteger& idx, vm_view state) {
        { converter<std::remove_cvref_t<T>>::From(state, idx, t) };
    };

////////////////////////////////////////////////////////////

enum class type : u8 {
    Null,
    Integer,
    Float,
    Boolean,
    String,
    Table,
    Array,
    Userdata,
    Closure,
    NativeClosure,
    Generator,
    UserPointer,
    Thread,
    Class,
    Instance,
    WeakReference
};

////////////////////////////////////////////////////////////

enum class generator_status : u8 {
    Ok,
    Suspended,
    Dead,
    Error
};

////////////////////////////////////////////////////////////

struct function_info {
    void*     FuncID {nullptr};
    string    Name;
    string    Source;
    SQInteger Line {0};
};

////////////////////////////////////////////////////////////

struct stack_info {
    string    FuncName;
    string    Source;
    SQInteger Line {0};
};

////////////////////////////////////////////////////////////

enum class debug_event : char {
    Call   = 'c',
    Return = 'r',
    Line   = 'l'
};

////////////////////////////////////////////////////////////

class TCOB_API stack_guard final : public non_copyable {
public:
    explicit stack_guard(HSQUIRRELVM vm);
    ~stack_guard();

    auto get_top() const -> SQInteger;

private:
    HSQUIRRELVM _vm;
    SQInteger   _oldTop;
};

////////////////////////////////////////////////////////////

class TCOB_API vm_view final {
public:
    enum class status : u8 {
        Idle,
        Running,
        Suspended
    };

    explicit vm_view(HSQUIRRELVM vm);

    auto create_stack_guard [[nodiscard]] () const -> stack_guard;

    template <typename... T>
    void push_convert(T&&... t) const;

    template <ConvertibleFrom T>
    auto pull_convert(SQInteger& idx, T& t) const -> bool;

    template <ConvertibleFrom T>
    auto pull_convert_idx(SQInteger idx, T& t) const -> bool;

    auto is_array(SQInteger idx) const -> bool;
    auto is_bool(SQInteger idx) const -> bool;
    auto is_function(SQInteger idx) const -> bool;
    auto is_closure(SQInteger idx) const -> bool;
    auto is_nativeclosure(SQInteger idx) const -> bool;
    auto is_integer(SQInteger idx) const -> bool;
    auto is_number(SQInteger idx) const -> bool;
    auto is_string(SQInteger idx) const -> bool;
    auto is_table(SQInteger idx) const -> bool;
    auto is_null(SQInteger idx) const -> bool;
    auto is_generator(SQInteger idx) const -> bool;
    auto is_thread(SQInteger idx) const -> bool;
    auto is_userpointer(SQInteger idx) const -> bool;
    auto is_userdata(SQInteger idx) const -> bool;
    auto is_class(SQInteger idx) const -> bool;
    auto is_instance(SQInteger idx) const -> bool;

    auto get(SQInteger idx) const -> bool;
    auto get_bool(SQInteger idx) const -> bool;
    auto get_integer(SQInteger idx) const -> SQInteger;
    auto get_float(SQInteger idx) const -> f32;
    auto get_string(SQInteger idx) const -> char const*;
    void get_stackobj(SQInteger idx, HSQOBJECT* o);
    auto get_thread(SQInteger idx) const -> vm_view;
    auto get_userpointer(SQInteger idx, void** p) const -> bool;
    auto get_userdata(SQInteger idx, void** p, void** typetag) const -> bool;
    auto get_class(SQInteger idx) const -> bool;

    auto new_slot(SQInteger idx, bool isStatic) const -> bool;
    auto set(SQInteger idx) const -> bool;

    auto get_type(SQInteger idx) const -> type;
    auto get_size(SQInteger idx) const -> SQInteger;
    auto get_top() const -> SQInteger;

    auto get_function_info(SQInteger level) const -> function_info;
    auto stack_infos(SQInteger level) const -> stack_info;
    auto get_local(SQUnsignedInteger level, SQUnsignedInteger nseq) const -> string;

    auto next(SQInteger idx) const -> bool;

    void push(SQInteger idx) const;
    void push_bool(bool val) const;
    void push_integer(SQInteger val) const;
    void push_float(f32 val) const;
    void push_null() const;
    void push_object(HSQOBJECT* po) const;
    void push_string(string_view val) const;
    void push_roottable() const;
    void push_userpointer(void* ptr) const;
    void push_registrytable() const;

    auto array_append(SQInteger idx) const -> bool;
    auto array_insert(SQInteger idx, SQInteger dest) const -> bool;

    void pop(SQInteger count) const;
    void poptop() const;
    void remove(SQInteger idx) const;

    void new_array(SQInteger size) const;
    void new_table() const;
    void new_table(SQInteger size) const;
    void new_closure(SQFUNCTION func, SQUnsignedInteger freeVars) const;
    auto new_thread(SQInteger initialstacksize) const -> vm_view;
    auto new_userdata(SQUnsignedInteger size) const -> void*;
    auto new_class(bool hasbase) const -> bool;
    auto create_instance(SQInteger idx) const -> bool;

    auto set_typetag(SQInteger idx, void* typetag) const -> bool;

    auto set_delegate(SQInteger idx) const -> bool;
    auto get_delegate(SQInteger idx) const -> bool;

    auto set_closure_root(SQInteger idx) const -> bool;
    auto get_closure_root(SQInteger idx) const -> bool;
    auto bind_env(SQInteger idx) const -> bool;

    auto raw_get(SQInteger idx) const -> bool;
    auto raw_set(SQInteger idx) const -> bool;

    void add_ref(HSQOBJECT* po) const;
    auto release(HSQOBJECT* po) const -> bool;

    auto resume(bool retVal) const -> bool;

    auto suspend_vm() const -> bool;
    auto wakeup_vm(bool resumedret, bool retval) const -> bool;
    auto get_vm_state() const -> status;

    void reset_object(HSQOBJECT* po) const;

    void throw_error(string const& message) const;
    void reset_error() const;
    auto has_error() const -> bool;
    auto get_error() const -> string;

    auto call(SQInteger params, bool retVal, bool raiseError) const -> std::optional<error_code>;

    void enable_debug_info(bool enable) const;

    auto compile_buffer(string_view script, string const& name) const -> std::optional<error_code>;

    void set_print_func(SQPRINTFUNCTION printfunc, SQPRINTFUNCTION errfunc) const;
    void set_compiler_errorhandler(SQCOMPILERERROR func) const;
    void set_errorhandler() const;
    void set_releasehook(SQInteger idx, SQRELEASEHOOK hook) const;
    void set_native_debughook(SQDEBUGHOOK hook) const;

    auto get_foreign_ptr() const -> void*; // used for hook
    void set_foreign_ptr(void* ptr) const; // used for hook

    auto cmp() const -> SQInteger;

    void register_iolib() const;
    void register_bloblib() const;
    void register_mathlib() const;
    void register_systemlib() const;
    void register_stringlib() const;

    auto static NewVM() -> HSQUIRRELVM;
    void close() const;

    auto is_valid() const -> bool;

    auto get_stack_types() const -> std::vector<type>;

private:
    template <ConvertibleFrom T>
    auto convert_from(SQInteger& idx, T& value) const -> bool;

    template <ConvertibleTo T>
    void convert_to(T const& value) const;

    template <ConvertibleTo T>
    void convert_to(T&& value) const;

    template <ConvertibleTo T>
    void convert_to(T& value) const;

    HSQUIRRELVM _vm {};
};

}

    #include "Squirrel.inl"

#endif
