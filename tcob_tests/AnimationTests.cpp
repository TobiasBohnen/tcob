#include "tests.hpp"

TEST_CASE("GFX.Animation.PlaybackModeNormal")
{
    FrameAnimation ani;
    ani.Frames = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };
    ani.Duration = 100;
    ani.Mode = AnimationPlaybackMode::Normal;

    REQUIRE(ani.get_frame(100 / 16.f * 0) == "0");
    REQUIRE(ani.get_frame(100 / 16.f * 1) == "1");
    REQUIRE(ani.get_frame(100 / 16.f * 2) == "2");
    REQUIRE(ani.get_frame(100 / 16.f * 3) == "3");
    REQUIRE(ani.get_frame(100 / 16.f * 4) == "4");
    REQUIRE(ani.get_frame(100 / 16.f * 5) == "5");
    REQUIRE(ani.get_frame(100 / 16.f * 6) == "6");
    REQUIRE(ani.get_frame(100 / 16.f * 7) == "7");
    REQUIRE(ani.get_frame(100 / 16.f * 8) == "8");
    REQUIRE(ani.get_frame(100 / 16.f * 9) == "9");
    REQUIRE(ani.get_frame(100 / 16.f * 10) == "10");
    REQUIRE(ani.get_frame(100 / 16.f * 11) == "11");
    REQUIRE(ani.get_frame(100 / 16.f * 12) == "12");
    REQUIRE(ani.get_frame(100 / 16.f * 13) == "13");
    REQUIRE(ani.get_frame(100 / 16.f * 14) == "14");
    REQUIRE(ani.get_frame(100 / 16.f * 15) == "15");

    REQUIRE(ani.get_frame(100 / 16.f * 16) == "15");
    REQUIRE(ani.get_frame(100 / 16.f * 17) == "15");
}

TEST_CASE("GFX.Animation.PlaybackModeLooped")
{
    FrameAnimation ani;
    ani.Frames = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };
    ani.Duration = 100;
    ani.Mode = AnimationPlaybackMode::Looped;

    for (int i { 0 }; i < 256; i++) {
        REQUIRE(ani.get_frame(100 / 16.f * i) == std::to_string(i % 16));
    }

    REQUIRE(ani.get_frame(100 / 16.f * 0) == "0");
    REQUIRE(ani.get_frame(100 / 16.f * 1) == "1");
    REQUIRE(ani.get_frame(100 / 16.f * 2) == "2");
    REQUIRE(ani.get_frame(100 / 16.f * 3) == "3");
    REQUIRE(ani.get_frame(100 / 16.f * 4) == "4");
    REQUIRE(ani.get_frame(100 / 16.f * 5) == "5");
    REQUIRE(ani.get_frame(100 / 16.f * 6) == "6");
    REQUIRE(ani.get_frame(100 / 16.f * 7) == "7");
    REQUIRE(ani.get_frame(100 / 16.f * 8) == "8");
    REQUIRE(ani.get_frame(100 / 16.f * 9) == "9");
    REQUIRE(ani.get_frame(100 / 16.f * 10) == "10");
    REQUIRE(ani.get_frame(100 / 16.f * 11) == "11");
    REQUIRE(ani.get_frame(100 / 16.f * 12) == "12");
    REQUIRE(ani.get_frame(100 / 16.f * 13) == "13");
    REQUIRE(ani.get_frame(100 / 16.f * 14) == "14");
    REQUIRE(ani.get_frame(100 / 16.f * 15) == "15");
    REQUIRE(ani.get_frame(100 / 16.f * 16) == "0");
    REQUIRE(ani.get_frame(100 / 16.f * 17) == "1");
    REQUIRE(ani.get_frame(100 / 16.f * 18) == "2");
    REQUIRE(ani.get_frame(100 / 16.f * 19) == "3");
    REQUIRE(ani.get_frame(100 / 16.f * 20) == "4");
    REQUIRE(ani.get_frame(100 / 16.f * 21) == "5");
    REQUIRE(ani.get_frame(100 / 16.f * 22) == "6");
    REQUIRE(ani.get_frame(100 / 16.f * 23) == "7");
    REQUIRE(ani.get_frame(100 / 16.f * 24) == "8");
    REQUIRE(ani.get_frame(100 / 16.f * 25) == "9");
    REQUIRE(ani.get_frame(100 / 16.f * 26) == "10");
    REQUIRE(ani.get_frame(100 / 16.f * 27) == "11");
    REQUIRE(ani.get_frame(100 / 16.f * 28) == "12");
    REQUIRE(ani.get_frame(100 / 16.f * 29) == "13");
    REQUIRE(ani.get_frame(100 / 16.f * 30) == "14");
    REQUIRE(ani.get_frame(100 / 16.f * 31) == "15");
    REQUIRE(ani.get_frame(100 / 16.f * 32) == "0");
}

TEST_CASE("GFX.Animation.PlaybackModeReversed")
{
    FrameAnimation ani;
    ani.Frames = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };
    ani.Duration = 100;
    ani.Mode = AnimationPlaybackMode::Reversed;

    REQUIRE(ani.get_frame(100 / 16.f * 0) == "15");
    REQUIRE(ani.get_frame(100 / 16.f * 1) == "14");
    REQUIRE(ani.get_frame(100 / 16.f * 2) == "13");
    REQUIRE(ani.get_frame(100 / 16.f * 3) == "12");
    REQUIRE(ani.get_frame(100 / 16.f * 4) == "11");
    REQUIRE(ani.get_frame(100 / 16.f * 5) == "10");
    REQUIRE(ani.get_frame(100 / 16.f * 6) == "9");
    REQUIRE(ani.get_frame(100 / 16.f * 7) == "8");
    REQUIRE(ani.get_frame(100 / 16.f * 8) == "7");
    REQUIRE(ani.get_frame(100 / 16.f * 9) == "6");
    REQUIRE(ani.get_frame(100 / 16.f * 10) == "5");
    REQUIRE(ani.get_frame(100 / 16.f * 11) == "4");
    REQUIRE(ani.get_frame(100 / 16.f * 12) == "3");
    REQUIRE(ani.get_frame(100 / 16.f * 13) == "2");
    REQUIRE(ani.get_frame(100 / 16.f * 14) == "1");
    REQUIRE(ani.get_frame(100 / 16.f * 15) == "0");

    REQUIRE(ani.get_frame(100 / 16.f * 16) == "0");
    REQUIRE(ani.get_frame(100 / 16.f * 17) == "0");
}

TEST_CASE("GFX.Animation.PlaybackModeReversedLooped")
{
    FrameAnimation ani;
    ani.Frames = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };
    ani.Duration = 100;
    ani.Mode = AnimationPlaybackMode::ReversedLooped;

    for (int i { 0 }; i < 256; i++) {
        REQUIRE(ani.get_frame(100 / 16.f * i) == std::to_string(15 - (i % 16)));
    }

    REQUIRE(ani.get_frame(100 / 16.f * 0) == "15");
    REQUIRE(ani.get_frame(100 / 16.f * 1) == "14");
    REQUIRE(ani.get_frame(100 / 16.f * 2) == "13");
    REQUIRE(ani.get_frame(100 / 16.f * 3) == "12");
    REQUIRE(ani.get_frame(100 / 16.f * 4) == "11");
    REQUIRE(ani.get_frame(100 / 16.f * 5) == "10");
    REQUIRE(ani.get_frame(100 / 16.f * 6) == "9");
    REQUIRE(ani.get_frame(100 / 16.f * 7) == "8");
    REQUIRE(ani.get_frame(100 / 16.f * 8) == "7");
    REQUIRE(ani.get_frame(100 / 16.f * 9) == "6");
    REQUIRE(ani.get_frame(100 / 16.f * 10) == "5");
    REQUIRE(ani.get_frame(100 / 16.f * 11) == "4");
    REQUIRE(ani.get_frame(100 / 16.f * 12) == "3");
    REQUIRE(ani.get_frame(100 / 16.f * 13) == "2");
    REQUIRE(ani.get_frame(100 / 16.f * 14) == "1");
    REQUIRE(ani.get_frame(100 / 16.f * 15) == "0");
    REQUIRE(ani.get_frame(100 / 16.f * 16) == "15");
    REQUIRE(ani.get_frame(100 / 16.f * 17) == "14");
    REQUIRE(ani.get_frame(100 / 16.f * 18) == "13");
    REQUIRE(ani.get_frame(100 / 16.f * 19) == "12");
    REQUIRE(ani.get_frame(100 / 16.f * 20) == "11");
    REQUIRE(ani.get_frame(100 / 16.f * 21) == "10");
    REQUIRE(ani.get_frame(100 / 16.f * 22) == "9");
    REQUIRE(ani.get_frame(100 / 16.f * 23) == "8");
    REQUIRE(ani.get_frame(100 / 16.f * 24) == "7");
    REQUIRE(ani.get_frame(100 / 16.f * 25) == "6");
    REQUIRE(ani.get_frame(100 / 16.f * 26) == "5");
    REQUIRE(ani.get_frame(100 / 16.f * 27) == "4");
    REQUIRE(ani.get_frame(100 / 16.f * 28) == "3");
    REQUIRE(ani.get_frame(100 / 16.f * 29) == "2");
    REQUIRE(ani.get_frame(100 / 16.f * 30) == "1");
    REQUIRE(ani.get_frame(100 / 16.f * 31) == "0");
    REQUIRE(ani.get_frame(100 / 16.f * 32) == "15");
}

TEST_CASE("GFX.Animation.PlaybackModeAlternated")
{
    FrameAnimation ani;
    ani.Frames = { "0", "1", "2", "3", "4" };
    ani.Duration = 90;
    ani.Mode = AnimationPlaybackMode::Alternated;

    REQUIRE(ani.get_frame(0) == "0");
    REQUIRE(ani.get_frame(10) == "1");
    REQUIRE(ani.get_frame(20) == "2");
    REQUIRE(ani.get_frame(30) == "3");
    REQUIRE(ani.get_frame(40) == "4");
    REQUIRE(ani.get_frame(50) == "3");
    REQUIRE(ani.get_frame(60) == "2");
    REQUIRE(ani.get_frame(70) == "1");
    REQUIRE(ani.get_frame(80) == "0");
    REQUIRE(ani.get_frame(90) == "0");
}

TEST_CASE("GFX.Animation.PlaybackModeAlternatedLooped")
{
    SECTION("Test1")
    {
        FrameAnimation ani;
        ani.Frames = { "0", "1", "2", "3", "4" };
        ani.Duration = 90;
        ani.Mode = AnimationPlaybackMode::AlternatedLooped;

        REQUIRE(ani.get_frame(0) == "0");
        REQUIRE(ani.get_frame(10) == "1");
        REQUIRE(ani.get_frame(20) == "2");
        REQUIRE(ani.get_frame(30) == "3");
        REQUIRE(ani.get_frame(40) == "4");
        REQUIRE(ani.get_frame(50) == "3");
        REQUIRE(ani.get_frame(60) == "2");
        REQUIRE(ani.get_frame(70) == "1");
        REQUIRE(ani.get_frame(80) == "0");
        REQUIRE(ani.get_frame(90) == "1");
        REQUIRE(ani.get_frame(100) == "2");
        REQUIRE(ani.get_frame(110) == "3");
        REQUIRE(ani.get_frame(120) == "4");
        REQUIRE(ani.get_frame(130) == "3");
        REQUIRE(ani.get_frame(140) == "2");
        REQUIRE(ani.get_frame(150) == "1");
        REQUIRE(ani.get_frame(160) == "0");
        REQUIRE(ani.get_frame(170) == "1");
        REQUIRE(ani.get_frame(180) == "2");
        REQUIRE(ani.get_frame(190) == "3");
    }
    SECTION("Test2")
    {
        FrameAnimation ani;
        ani.Frames = { "0", "1", "2" };
        ani.Duration = 50;
        ani.Mode = AnimationPlaybackMode::AlternatedLooped;

        REQUIRE(ani.get_frame(0) == "0");
        REQUIRE(ani.get_frame(10) == "1");
        REQUIRE(ani.get_frame(20) == "2");
        REQUIRE(ani.get_frame(30) == "1");
        REQUIRE(ani.get_frame(40) == "0");
        REQUIRE(ani.get_frame(50) == "1");
        REQUIRE(ani.get_frame(60) == "2");
        REQUIRE(ani.get_frame(70) == "1");
        REQUIRE(ani.get_frame(80) == "0");
        REQUIRE(ani.get_frame(90) == "1");
        REQUIRE(ani.get_frame(100) == "2");
        REQUIRE(ani.get_frame(110) == "1");
        REQUIRE(ani.get_frame(120) == "0");
        REQUIRE(ani.get_frame(130) == "1");
        REQUIRE(ani.get_frame(140) == "2");
        REQUIRE(ani.get_frame(150) == "1");
        REQUIRE(ani.get_frame(160) == "0");
        REQUIRE(ani.get_frame(170) == "1");
        REQUIRE(ani.get_frame(180) == "2");
        REQUIRE(ani.get_frame(190) == "1");
    }
}