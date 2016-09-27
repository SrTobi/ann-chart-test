#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <cassert>
#include <memory>
#include "abstract_game.hpp"
#include "chart_game.hpp"
#include "ai_game.hpp"
/*



void key_pressed( sf::Keyboard::Key code )
{
	if(GCurrentAction == Action_Nothing)
	{
		if(code == sf::Keyboard::Up)
		{
			GEntrance = GAdapter->cur_value();
			GCurrentAction = Action_Long;
		}else if(code == sf::Keyboard::Down)
		{
			GEntrance = GAdapter->cur_value();
			GCurrentAction = Action_Short;
		}
	}else if(code == sf::Keyboard::Space)
	{
		GWin += (GAdapter->cur_value() - GEntrance) * (GCurrentAction == Action_Short? -1.0f : 1.0f);
		GCurrentAction = Action_Nothing;
	}
}*/


class Application
{
public:
	Application()
	{
		mGame = GetAiGame();
	}


	~Application()
	{
	}


	/*void move(float dt)
	{
		if(GAdapter->add_tick(dt * 1000.0f))
			GChart->notifiy_update();
	}


	void draw()
	{


		GChart->render(GWindow);

		if(GCurrentAction != Action_Nothing)
		{
			sf::Vector2f size(1.0f, GAdapter->max_y() - GAdapter->min_y());
			sf::Vector2f center(0.5f,
				GAdapter->min_y() + size.y * 0.5f);

			GWindow.setView(sf::View(center, size));

			sf::Color color = (GCurrentAction == Action_Long? sf::Color::Green : sf::Color::Red);

			sf::Vertex line[] =
			{
				sf::Vertex(sf::Vector2f(0, -GEntrance), color),
				sf::Vertex(sf::Vector2f(1, -GEntrance), color)
			};

			GWindow.draw(line, 2, sf::Lines);
		}
	}*/


	int run()
	{
		using namespace std::chrono;

		// create the window
		mWindow.create(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, sf::ContextSettings(32));
		mWindow.setVerticalSyncEnabled(true);

		const long ft_update_interval = 100;
		high_resolution_clock clock;

		// run the main loop
		bool running = true;
		long time_left_for_ft_update = ft_update_interval;
		long dt = 10;
		while (running && mWindow.isOpen())
		{
			time_point<std::chrono::high_resolution_clock> start, end;

			if(mGame)
			{
				auto next_game = mGame->next_game();
				if(next_game)
					mGame = next_game;
			}

			// handle events
			sf::Event event;
			while (mWindow.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					// end the program
					running = false;
				}
				else if (event.type == sf::Event::Resized)
				{
					// adjust the viewport when the window is resized
					//mChartRenderer->render_rect(sf::FloatRect(0, 0, float(event.size.width), float(event.size.height)));
					if(mGame)
						mGame->window_resized(sf::Vector2u(event.size.width, event.size.height));

				}else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				{
					mWindow.close();
				}else if(event.type == sf::Event::KeyPressed)
				{
					if(mGame)
						mGame->key_pressed(event.key.code);
				}
			}
			start = clock.now();

			mWindow.clear();
			if(mGame)
			{
				mGame->move(float(dt) / 1000.0f);
				mGame->render(mWindow);
			}

			// end the current frame (internally swaps the front and back buffers)
			mWindow.display();

			end = clock.now();

			dt = (long)std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

			if((time_left_for_ft_update -= dt) < 0) {
				mWindow.setTitle( "Test [Frame time: "
								+ std::to_string(dt)
								+ "]");
				time_left_for_ft_update = ft_update_interval;
			}
		}

		if(mGame)
			mGame->close_game();

		return 0;
	}

private:
	sf::RenderWindow mWindow;
	AbstractGame* mGame;
};



int main()
{

	Application app;

	return app.run();
}