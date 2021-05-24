#include "tests.hpp"

TEST_CASE("GFX.Text.Tokenizer")
{
    auto font { ResourcePtr<Font> { nullptr } };
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "   ", font);
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Whitespace);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "   a", font);
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, " a ", font);
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Whitespace);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "a b c", font);
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[3].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[4].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "abc", font);
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "öäößह", font);
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "abc def", font);
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Text);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "a\n b   \nc\n ", font);
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
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "abc def {color=red}", font);
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[2].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[3].Type == TextFormatter::ShaperTokenType::Whitespace);
        REQUIRE(tokens[4].Type == TextFormatter::ShaperTokenType::Command);
    }
    {
        std::vector<TextFormatter::ShaperToken> tokens;
        TextFormatter::shape(tokens, "a{  {", font);
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[1].Type == TextFormatter::ShaperTokenType::Text);
        REQUIRE(tokens[1].Text == "{");
    }
}