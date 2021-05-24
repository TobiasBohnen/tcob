#include "tests.hpp"

static_assert(std::is_copy_constructible_v<Sprite>);
static_assert(std::is_copy_assignable_v<Sprite>);
static_assert(std::is_move_constructible_v<Sprite>);
static_assert(std::is_move_assignable_v<Sprite>);

TEST_CASE("GFX.SpriteBatch.Creating")
{
    SpriteBatch _batch;
    _batch.create_sprite();
    REQUIRE(_batch.sprite_count() == 1);
    auto& sprite1 { _batch.at(0) };
    REQUIRE(sprite1.id() == 1);
}
TEST_CASE("GFX.SpriteBatch.Adding")
{
    SpriteBatch _batch;
    Sprite sprite0 {};
    _batch.add_sprite(sprite0);
    REQUIRE(_batch.sprite_count() == 1);
    auto& sprite1 { _batch.at(0) };
    REQUIRE(sprite1.id() == sprite0.id());
}

TEST_CASE("GFX.SpriteBatch.FindIf")
{
    SpriteBatch _batch;
    int id = _batch.create_sprite().id();
    _batch.create_sprite();
    _batch.create_sprite();
    REQUIRE(_batch.sprite_count() == 3);

    auto sprite1 { _batch.find_if([id](const auto& s) { return s.id() == id; }) };
    REQUIRE(sprite1 != nullptr);
    REQUIRE(sprite1->id() == id);

    auto sprite2 { _batch.find_if([](const auto& s) { return s.id() == 0; }) };
    REQUIRE(sprite2 == nullptr);
}

TEST_CASE("GFX.SpriteBatch.FindIfNot")
{
    SpriteBatch _batch;
    auto& sprite0 { _batch.create_sprite() };
    sprite0.hide();
    int id = sprite0.id();

    auto sprite1 { _batch.find_if_not([id](const auto& s) { return s.id() == id; }) };
    REQUIRE(sprite1 == nullptr);

    auto sprite2 { _batch.find_if_not(
        [](const auto& s) {
            return s.is_visible();
        }) };
    REQUIRE(sprite2 != nullptr);
    REQUIRE(sprite2->id() == sprite0.id());
    REQUIRE(sprite2 == &sprite0);
}