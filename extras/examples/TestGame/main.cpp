#include "StartScene.hpp"

#include <SDL.h>
#include <tcob/tcob.hpp>

int main(int argc, char* argv[])
{
    tcob::Game game { argv[0], "TestGame" };
    game.push_scene<StartScene>();
    game.start();
    return 0;
}