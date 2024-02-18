#pragma once
#include <utility>

#include "tests.hpp"

using namespace tcob::scripting;

template <typename... Args, typename R, typename T>
auto constexpr resolve_overload(R (T::*ptr)(Args...))
{
    return ptr;
}

template <typename R, typename T>
auto constexpr resolve_overload(R (T::*ptr)())
{
    return ptr;
}

class TestScriptClassBase {
public:
    virtual ~TestScriptClassBase() = default;
    auto baseMethod() const -> int { return _x; }
    auto virtual virtualMethod() -> int { return 1; }
    auto virtual abstractMethod() -> int = 0;

private:
    int _x {21};
};
class TestScriptClass : public TestScriptClassBase {
public:
    static inline int ObjCount;

    using value_type = int;
    using size_type  = int;
    auto operator[](int) -> int& { return _value; }

    TestScriptClass()
    {
        ObjCount++;
    }

    TestScriptClass(int age)
        : _value(age)
    {
        ObjCount++;
    }

    TestScriptClass(int age, f32 x)
        : _value(age * (int)x)
    {
        ObjCount++;
    }

    ~TestScriptClass() override
    {
        ObjCount--;
    }

    void set_value(int x)
    {
        _value = x;
    }
    auto get_value() const -> int
    {
        return _value;
    }

    i32       FieldValue {};
    prop<i32> PropertyValue;
    bool      Closed {false};

    auto get_map() -> std::map<std::string, int>*
    {
        return &_testMap;
    }
    void ptr(TestScriptClass* c)
    {
        _value = c->_value;
    }
    auto foo(std::string const& test, int x, bool) -> int
    {
        return (int)test.length() * x;
    }
    void bar(bool, std::string const&, int) { }

    auto add_value(int x) const -> int
    {
        return _value + x;
    }

    auto overload(i32 i, std::pair<f32, std::string> const& p, f32 f) -> f32
    {
        return static_cast<f32>(i) * p.first * f;
    }
    auto overload(std::tuple<f32, i32, std::string> const& tup) -> f32
    {
        return std::get<0>(tup) * static_cast<f32>(std::get<1>(tup));
    }
    auto overload(std::vector<f32> const& vec) -> f32
    {
        return static_cast<f32>(vec.size());
    }
    auto overload(f32 f, int i) -> f32
    {
        return f * static_cast<f32>(i) * 3;
    }
    auto overload(int i, f32 f) -> f32
    {
        return f * static_cast<f32>(i) * 25;
    }
    auto overload() -> f32
    {
        return 25;
    }

    auto virtualMethod() -> int override { return 84; }
    auto abstractMethod() -> int override { return 42; }

    std::map<std::string, int> _testMap;
    int                        _value = 0;
};

class TestScriptClassSub final : public TestScriptClass {
};

inline auto operator==(TestScriptClass const& left, TestScriptClass const& right) -> bool
{
    return (left._value == right._value);
}
inline auto testFuncWrapperObj(TestScriptClass const* tsc) -> int
{
    return tsc->get_value();
}

struct op_test_class {
    static inline int ObjCount;
    op_test_class(int v)
        : value(v)
    {
        ObjCount++;
    }
    ~op_test_class()
    {
        ObjCount--;
    }

    int value {0};
};
inline auto operator+(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value + right.value};
}
inline auto operator-(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value - right.value};
}
inline auto operator/(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value / right.value};
}
inline auto operator*(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value * right.value};
}
inline auto operator-(op_test_class const& right) -> op_test_class
{
    return {-right.value};
}
inline auto operator==(op_test_class const& left, op_test_class const& right) -> bool
{
    return left.value == right.value;
}
inline auto operator<=>(op_test_class const& left, op_test_class const& right)
{
    return left.value <=> right.value;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class Player {
public:
    // Constructor
    Player(std::string name, int health, int mana)
        : _name(std::move(name))
        , _health(health)
        , _mana(mana)
    {
    }

    auto get_health() const -> int
    {
        return _health;
    }

    void set_health(int amount)
    {
        _health = amount;
    }

    void take_damage(int damage)
    {
        _health -= damage;
    }

    auto get_mana() const -> int
    {
        return _mana;
    }

    void set_mana(int amount)
    {
        _mana = amount;
    }

    void use_mana(int amount)
    {
        if (_mana >= amount) {
            _mana -= amount;
        }
    }

    auto get_experience() const -> int
    {
        return _experience;
    }

    void gain_experience(int points)
    {
        _experience += points;
        if (_experience >= _level * 100) {
            _level++;
        }
    }

    void add_to_inventory(std::string const& item)
    {
        _inventory.emplace_back(item);
    }

    void add_to_inventory(int item)
    {
        _inventory.emplace_back(item);
    }

    auto get_inventory_size() const -> i32
    {
        return static_cast<i32>(_inventory.size());
    }

private:
    std::string                                 _name;
    int                                         _health;
    int                                         _mana;
    int                                         _level {1};
    int                                         _experience {0};
    std::vector<std::variant<std::string, int>> _inventory;
};
