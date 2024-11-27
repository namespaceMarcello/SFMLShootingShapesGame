#include "Vec2.h"
#include "Game.h"

#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>



int main()
{
   
	Game game("config.txt");
	game.run();

}