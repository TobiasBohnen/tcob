#pragma once
#include "tests.hpp"
inline std::ostream& operator<<(std::ostream& os, LuaResultState bar)
{
    switch (bar) {
    case tcob::LuaResultState::Ok:
        return os << "Ok";
    case tcob::LuaResultState::Yielded:
        return os << "Yielded";
    case tcob::LuaResultState::Undefined:
        return os << "Undefined";
    case tcob::LuaResultState::TypeMismatch:
        return os << "TypeMismatch";
    case tcob::LuaResultState::NonTableIndex:
        return os << "NonTableIndex";
    case tcob::LuaResultState::RuntimeError:
        return os << "RuntimeError";
    case tcob::LuaResultState::MemAllocError:
        return os << "MemAllocError";
    case tcob::LuaResultState::SyntaxError:
        return os << "SyntaxError";
    default:
        return os;
    }
}

inline std::ostream& operator<<(std::ostream& os, LuaCoroutineState bar)
{
    switch (bar) {
    case tcob::LuaCoroutineState::Ok:
        return os << "Ok";
    case tcob::LuaCoroutineState::Suspended:
        return os << "Suspended";
    case tcob::LuaCoroutineState::Error:
        return os << "Error";
    default:
        return os;
    }
}

template <typename... Args, typename C, typename R>
constexpr auto overload(R (C::*ptr)(Args...))
{
    return ptr;
}

struct foo {
    int x = 0;
    int y = 0;
    int z = 0;
};

namespace tcob {
template <>
struct LuaConverter<foo> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const LuaState& ls, i32 idx) -> bool
    {
        LuaTable lt { ls, idx };
        return lt.has("x") && lt.has("y") && lt.has("z");
    }

    static auto FromLua(const LuaState& ls, i32&& idx, foo& value) -> bool
    {
        if (ls.is_table(idx)) {
            LuaTable lt { ls, idx++ };

            value.x = lt["x"];
            value.y = lt["y"];
            value.z = lt["z"];
        }
        return true;
    }

    static void ToLua(const LuaState& ls, const foo& value)
    {
        ls.new_table();
        LuaTable lt { ls, -1 };

        lt["x"] = value.x;
        lt["y"] = value.y;
        lt["z"] = value.z;
    }
};
}

static string testfuncstr()
{
    return "huhu";
}

static float testfuncfloat()
{
    return 4.2f;
}
static float testfuncpair(const std::pair<i32, f32>& p)
{
    return p.first * p.second;
}
static float testfuncfloat2(LuaResult<float> f, LuaResult<float> x, int i)
{
    return f.Value * x.Value * i;
}

static LuaTable openrequire(LuaScript& state, const string& name)
{
    string libname = name + ".lua";
    return state.run_file<LuaTable>(libname, -1).Value;
}