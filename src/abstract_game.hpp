#pragma once
#ifndef _GAME_HPP 
#define _GAME_HPP 

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <memory>

class AbstractGame
{
public:
	virtual void move(float dt) = 0;
	virtual void render(sf::RenderTarget& target) = 0;

	virtual void window_resized(sf::Vector2u& size) = 0;
	virtual void key_pressed(sf::Keyboard::Key key) = 0;
	virtual AbstractGame* next_game() = 0;
	virtual void close_game() = 0;
private:
};

#endif