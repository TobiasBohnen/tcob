#include "tests.hpp"

TEST_CASE("GFX.Text.Tokenizer")
{
    auto font { ResourcePtr<Font> { nullptr } };
    {
        auto tokens { TextFormatter::shape("   ", font) };
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Whitespace);
    }
    {
        auto tokens { TextFormatter::shape("   a", font) };
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        auto tokens { TextFormatter::shape(" a ", font) };
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Whitespace);
    }
    {
        auto tokens { TextFormatter::shape("a b c", font) };
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[3].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[4].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        auto tokens { TextFormatter::shape("abc", font) };
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        auto tokens { TextFormatter::shape("öäößह", font) };
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        auto tokens { TextFormatter::shape("abc def", font) };
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        auto tokens { TextFormatter::shape("a\n b   \nc\n ", font) };
        REQUIRE(tokens.size() == 9);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Newline);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[3].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[4].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[5].Type == TextFormatter::ShaperTokenType::Newline);
        REQUIRE(tokens[6].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[7].Type == TextFormatter::ShaperTokenType::Newline);
        REQUIRE(tokens[8].Type == TextFormatter::ShaperTokenType::Whitespace);
    }
    {
        auto tokens { TextFormatter::shape("abc def {color=red}", font) };
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[3].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[4].Type == TextFormatter::ShaperTokenType::Command);
    }
    {
        auto tokens { TextFormatter::shape("a{  {", font) };
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Text == "{");
    }
}