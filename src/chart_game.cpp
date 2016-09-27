#include "chart_game.hpp"

#include "chart_data.hpp"
#include "chart_model.hpp"
#include "chart_renderer.hpp"
#include "realtime_chart.hpp"
#include "chart_trader.hpp"

static const float MIN_CHART_VALUE = 0;
static const float MAX_CHART_VALUE = 10;
static const float CHART_IN_SECONDS = 60.0f;
static const float TICKS_PER_SECOND = 30.0f;
static const float CHART_VOLATILITY = 0.24f;


class GameChartAdapter: public ChartData
{
public:
	GameChartAdapter(RealtimeChart* model)
		: mModel(model)
	{
	}


	virtual float min_x() const
	{
		return std::max(max_x() - 5.0f, 0.0f);
	}

	virtual float min_y() const
	{
		return mModel->min_value();
	}

	virtual float max_x() const
	{
		return std::max(std::min(mModel->current_time(), CHART_IN_SECONDS), 5.0f);
	}

	virtual float max_y() const
	{
		return mModel->max_value();
	}

	virtual std::size_t data_count() const
	{
		return std::min(mModel->max_ticks(), mModel->current_tick() + 1);
	}

	virtual void data_point(int idx, float* x, float* y ) const
	{
		*x = float(idx) / mModel->ticks_per_second();
		*y = mModel->tick_value(idx);
	}
private:
	std::size_t mTime;
	RealtimeChart* mModel;
};


class ChartGame: public AbstractGame
{
public:
	ChartGame()
	{
		mChartModel.reset(new ChartModel(MIN_CHART_VALUE, MAX_CHART_VALUE, 0.25f, std::size_t(CHART_IN_SECONDS * TICKS_PER_SECOND)));
		mChart.reset(new RealtimeChart(mChartModel.get(), TICKS_PER_SECOND));
		mChartAdapter.reset(new GameChartAdapter(mChart.get()));
		mChartRenderer.reset(new ChartRenderer(mChartAdapter.get(), sf::FloatRect()));

		mTrader.reset(new ChartTrader(mChart.get(), 0.0f));
	}

	~ChartGame()
	{

	}

	virtual void move( float dt )
	{
		if(mChart->walk_time(dt))
			mChartRenderer->notifiy_update();
	}

	void render_order(sf::RenderTarget& target, const Order& order, const sf::Color& color)
	{
		if(order.active())
		{
			sf::Vertex line[] =
			{
				sf::Vertex(sf::Vector2f(0, mChartAdapter->max_y() - order.entrance()), color),
				sf::Vertex(sf::Vector2f(1, mChartAdapter->max_y() - order.entrance()), color)
			};
			target.draw(line, 2, sf::Lines);
		}
	}

	virtual void render( sf::RenderTarget& target )
	{

		mChartRenderer->render(target);

		if(mTrader->is_trading())
		{
			sf::Vector2f size(1.0f, mChartAdapter->max_y() - mChartAdapter->min_y());
			sf::Vector2f center(0.5f,
				mChartAdapter->min_y() + size.y * 0.5f);

			target.setView(sf::View(center, size));

			render_order(target, mTrader->long_order(), sf::Color::Green);
			render_order(target, mTrader->short_order(), sf::Color::Red);

		}
	}

	virtual void window_resized( sf::Vector2u& size )
	{
		mChartRenderer->render_rect(sf::FloatRect(0, 0, float(size.x), float(size.y)));
	}

	virtual void key_pressed( sf::Keyboard::Key key )
	{
		if(key == sf::Keyboard::Space)
		{
			if(mTrader->long_order().active())
			{
				mTrader->long_order().leave();
			}else if(mTrader->short_order().active())
			{
				mTrader->short_order().leave();
			}
		} else if(!mTrader->is_trading())
		{
			if(key == sf::Keyboard::Up)
			{
				mTrader->long_order().breach();
			} else if(key == sf::Keyboard::Down)
			{
				mTrader->short_order().breach();
			}
		}
	}

	virtual AbstractGame* next_game()
	{
		return nullptr;
	}


	virtual void close_game()
	{

	}
private:
	std::unique_ptr<GameChartAdapter> mChartAdapter;
	std::unique_ptr<ChartRenderer> mChartRenderer;
	std::unique_ptr<ChartModel> mChartModel;
	std::unique_ptr<RealtimeChart> mChart;
		
	std::unique_ptr<ChartTrader> mTrader;
};

AbstractGame* CreateChartGame()
{
	static ChartGame* game = new ChartGame();
	return game;
}


