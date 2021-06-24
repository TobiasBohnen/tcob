#pragma once
#include "tests.hpp"
class TestScriptClass {
public:
    inline static int ObjCount;

    using value_type = int;
    int& operator[](int idx)
    {
        return _importantValue;
    }

    TestScriptClass()
    {
        ObjCount++;
    }
    TestScriptClass(int age)
        : _importantValue(age)
    {
        ObjCount++;
    }
    TestScriptClass(int age, f32 x)
        : _importantValue(age * (int)x)
    {
        ObjCount++;
    }
    ~TestScriptClass()
    {
        ObjCount--;
    }

    void set_value(int x)
    {
        _importantValue = x;
    }

    const int get_value() const
    {
        return _importantValue;
    }

    map<string, int>* get_map()
    {
        return &_testMap;
    }

    void ptr(TestScriptClass* c)
    {
        _importantValue = c->_importantValue;
    }

    int foo(const std::string& test, int x, bool b)
    {
        return (int)test.length() * x;
    }

    void bar(bool b, const std::string& test, int x)
    {
    }

    int add_value(int x)
    {
        return _importantValue + x;
    }

    f32 overload(i32 x, const pair<f32, string>& vec, f32 y)
    {
        return 1;
    }

    f32 overload(const tuple<f32, i32, string>& vec)
    {
        return 2;
    }

    f32 overload(const vector<f32>& vec)
    {
        return 3;
    }

    f32 overload(f32 f, int x)
    {
        return 4;
    }

    f32 overload(int x, f32 f)
    {
        return 5;
    }

    map<string, int> _testMap;
    int _importantValue = 0;
};

class TestScriptClassSub : public TestScriptClass {
};

inline bool operator==(const TestScriptClass& left, const TestScriptClass& right)
{
    return (left._importantValue == right._importantValue);
}
inline bool operator!=(const TestScriptClass& left, const TestScriptClass& right)
{
    return (left._importantValue != right._importantValue);
}
inline static int testFuncWrapperObj(const TestScriptClass* tsc)
{
    return tsc->get_value();
}