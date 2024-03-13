#include "tests.hpp"

using namespace tcob::assets;

TEST_CASE("Core.Assets.IsValid")
{
    auto ptr = std::make_shared<i32>();
    auto ast = asset_ptr<i32>(std::make_shared<asset<i32>>("", ptr, nullptr));
    REQUIRE(ast);
    ptr = nullptr;
    REQUIRE_FALSE(ast);
}

struct Base {
    Base()          = default;
    virtual ~Base() = default;

    int Type {0};
};

struct Derived : public Base {
    Derived()
    {
        Type = 1;
        Count++;
    }
    ~Derived() override
    {
        Count--;
    }

    static inline int Count {0};
};
struct Foo { };
TEST_CASE("Core.Assets.Cast")
{
    {
        {
            auto            derivedObj      = std::make_shared<Derived>();
            auto            derivedAsset    = std::make_shared<asset<Derived>>("", derivedObj, nullptr);
            auto            derivedAssetPtr = asset_ptr<Derived> {derivedAsset};
            asset_ptr<Base> baseAssetPtr    = derivedAssetPtr;
            REQUIRE(Derived::Count == 1);
            REQUIRE(baseAssetPtr->Type == 1);
        }
        REQUIRE(Derived::Count == 0);
    }
    {
        {
            auto               derivedObj      = std::make_shared<Derived>();
            auto               baseAsset       = std::make_shared<asset<Base>>("", derivedObj, nullptr);
            auto               baseAssetPtr    = asset_ptr<Base> {baseAsset};
            asset_ptr<Derived> derivedAssetPtr = baseAssetPtr;
            REQUIRE(Derived::Count == 1);
            REQUIRE(derivedAssetPtr->Type == 1);
        }
        REQUIRE(Derived::Count == 0);
    }
}
