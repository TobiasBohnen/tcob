#pragma once
#include "tests.hpp"
inline std::ostream& operator<<(std::ostream& os, ResultState bar)
{
    switch (bar) {
    case tcob::lua::ResultState::Ok:
        return os << "Ok";
    case tcob::lua::ResultState::Yielded:
        return os << "Yielded";
    case tcob::lua::ResultState::Undefined:
        return os << "Undefined";
    case tcob::lua::ResultState::TypeMismatch:
        return os << "TypeMismatch";
    case tcob::lua::ResultState::NonTableIndex:
        return os << "NonTableIndex";
    case tcob::lua::ResultState::RuntimeError:
        return os << "RuntimeError";
    case tcob::lua::ResultState::MemAllocError:
        return os << "MemAllocError";
    case tcob::lua::ResultState::SyntaxError:
        return os << "SyntaxError";
    default:
        return os;
    }
}

inline std::ostream& operator<<(std::ostream& os, CoroutineState bar)
{
    switch (bar) {
    case tcob::lua::CoroutineState::Ok:
        return os << "Ok";
    case tcob::lua::CoroutineState::Suspended:
        return os << "Suspended";
    case tcob::lua::CoroutineState::Error:
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
struct Converter<foo> {
    static constexpr i32 StackSlots { 1 };

    static auto IsType(const State& ls, i32 idx) -> bool
    {
        Table lt { ls, idx };
        return lt.has("x") && lt.has("y") && lt.has("z");
    }

    static auto FromLua(const State& ls, i32&& idx, foo& value) -> bool
    {
        if (ls.is_table(idx)) {
            Table lt { ls, idx++ };

            value.x = lt["x"];
            value.y = lt["y"];
            value.z = lt["z"];
        }
        return true;
    }

    static void ToLua(const State& ls, const foo& value)
    {
        ls.new_table();
        Table lt { ls, -1 };

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
static float testfuncfloat2(Result<float> f, Result<float> x, int i)
{
    return f.Value * x.Value * i;
}

static Table openrequire(Script& state, const string& name)
{
    string libname = name + ".lua";
    return state.run_file<Table>(libname, -1).Value;
}