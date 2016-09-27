#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <numeric>
#include <condition_variable>
#include <iostream>
#include <array>
#include <iterator>
#include <cassert>
#include <list>
#include "ai_game.hpp"
#include "chart_data.hpp"
#include "chart_renderer.hpp"
#include "chart_trader.hpp"
#include "ann.hpp"
#include "chart_model.hpp"
#include "thread_pool.hpp"
#include "realtime_chart.hpp"

#define GEN_COUNT 100

ANNFormat<3, 4> AiFormat(5, 2);

typedef ANN<3, 4> MyANN;

static const float MIN_CHART_VALUE = 0;
static const float MAX_CHART_VALUE = 10;
static const float CHART_IN_SECONDS = 20.0f;
static const float TICKS_PER_SECOND = 30.0f;
static const float CHART_VOLATILITY = 0.24f;

struct PopulationStats
{
	float max_fitness;
	float avg_fitness;
	float min_fitness;
	std::size_t generation;
};

class Entity
{
public:
	Entity()
		: mANN(new MyANN(AiFormat))
		, mFitness(0)
	{
	}


	Entity(const std::shared_ptr<MyANN>& ann)
		: mANN(ann)
		, mFitness(0)
	{
	}

	float process(ChartModel* model)
	{
		TickChart chart(model);
		ChartTrader trader(&chart, 0.0f);

		int short_actions = 0;
		int long_actions = 0;

		MyANN::data_type data(AiFormat);

		while(!chart.is_done())
		{
			data.in[0] = chart.current_value();
			data.in[1] = trader.long_order().active()? trader.long_order().entrance() : -1.0f;
			data.in[2] = trader.short_order().active()? trader.short_order().entrance() : -1.0f;

			auto& output = mANN->process(data);
			float long_enter = output[0];
			float long_leave = output[1];
			float short_enter = output[2];
			float short_leave = output[3];
			
			if(long_leave > 0.5 && trader.long_order().active())
			{
				long_actions++;
				trader.long_order().leave();
			}else if(long_enter > 0.5 && !trader.long_order().active())
			{
				trader.long_order().breach();
			}

			if(short_leave > 0.5 && trader.short_order().active())
			{
				short_actions++;
				trader.short_order().leave();
			}else if(short_enter > 0.5 && !trader.short_order().active())
			{
				trader.short_order().breach();
			}

			chart.walk_tick();
		}

		mFitness = trader.capital();
		return mFitness;
	}

	float fitness() const
	{
		return mFitness;
	}

	bool operator <(const Entity& other)
	{
		return mFitness < other.mFitness;
	}

	const MyANN& ann() const
	{
		return *mANN;
	}

private:
	std::shared_ptr<MyANN> mANN;
	float mFitness;
};

static ThreadPool& GetPool()
{
	static ThreadPool Pool(4);
	return Pool;
}


class Generation
{
public:
	Generation(int pop_count)
		: mGenerationIndex(0)
	{
		while(pop_count--)
		{
			mEntities.push_back(Entity());
		}
	}

	Generation(unsigned seed, const std::unique_ptr<Generation>& old)
		: mGenerationIndex(old->mGenerationIndex + 1)
	{
		std::vector<Entity>& population = old->mEntities;
		auto pop_size = population.size();

		float acc_fitness = std::accumulate(population.begin(), population.end(), 0.0f, [](float acc, const Entity& e){return e.fitness() + acc;});

		float bounds[] = {0.0f, acc_fitness};
		std::sort(bounds, bounds + 2);

		std::default_random_engine generator(seed);
		std::uniform_real_distribution<float> selection_distribution(bounds[0], bounds[1]);
		std::normal_distribution<float> normal_distribution(0.0f, 0.15f);
		std::uniform_real_distribution<float> zeroone_distribution(0.0f, 1.0f);
		auto selection_rand = [&]() { return selection_distribution(generator); };
		auto normal_rand = [&]() { return normal_distribution(generator); };
		auto zeroone_rand = [&]() { return zeroone_distribution(generator); };

		while(pop_size--)
		{
			float selection = selection_rand();
			for(auto it = population.rbegin(); it != population.rend(); ++it)
			{
				selection -= it->fitness();
				if(selection <= 0)
				{
					const MyANN& ann = it->ann();
					auto genoms = ann.neuron_weights().clone();
					
					for(auto& genom : genoms)
						if(zeroone_rand() < 0.1f)
							genom += normal_rand();

					mEntities.push_back(Entity(std::make_shared<MyANN>(AiFormat, std::move(genoms))));
					break;
				}
			}
		}
	}

	void process(ChartModel* model)
	{
		float avg_fitness = 0.0f;

		std::vector<float> fitnesses(mEntities.size(), 0.0f);
		auto it = fitnesses.begin();
		for(auto& e : mEntities)
		{
			float& f = *it;
			GetPool().post([&e, model, &f]{f = e.process(model);});
			//f = e.process(model);
			++it;
		}
		assert(it == fitnesses.end());
		GetPool().complete();
		avg_fitness = std::accumulate(fitnesses.begin(), fitnesses.end(), 0.0f) / float(mEntities.size());

		std::sort(mEntities.begin(), mEntities.end());
		mStats.min_fitness = mEntities.front().fitness();
		mStats.avg_fitness = avg_fitness;
		mStats.max_fitness = mEntities.back().fitness();
		mStats.generation = mGenerationIndex;
	}

	PopulationStats stats() const
	{
		return mStats;
	}

private:
	int mGenerationIndex;
	std::vector<Entity> mEntities;
	PopulationStats mStats;
};


class AiTest
{
public:
	AiTest()
	{
	}

	void run()
	{
		mRunning = true;
		while (mRunning)
		{
			if(!mCurrentGeneration)
			{
				mCurrentGeneration.reset(new Generation(GEN_COUNT));
			}else{
				unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
				mCurrentGeneration.reset(new Generation(seed, mCurrentGeneration));
			}

			ChartModel model(MIN_CHART_VALUE, MAX_CHART_VALUE, 0.25f, std::size_t(CHART_IN_SECONDS * TICKS_PER_SECOND));
			mCurrentGeneration->process(&model);

			_add_stats(mCurrentGeneration->stats());
		}
	}

	void start()
	{
		mThread = std::thread(std::bind(&AiTest::run, this));
	}

	void stop()
	{
		mRunning = false;
		mThread.join();
	}

	std::vector<PopulationStats> get_new_fitness()
	{
		std::lock_guard<std::mutex> guard(mChartMutex);
		return std::move(mNewStats);
	}
private:
	void _add_stats(const PopulationStats& stats)
	{
		std::lock_guard<std::mutex> guard(mChartMutex);
		mNewStats.push_back(stats);
	}

private:
	bool  mRunning;
	std::unique_ptr<Generation> mCurrentGeneration;
	std::thread mThread;
	std::mutex mChartMutex;
	std::vector<PopulationStats> mNewStats;
};


class ChartAdapter: public ChartData
{
public:
	ChartAdapter(std::list<float>& model)
		: mModel(model)
	{
	}


	virtual float min_x() const
	{
		return std::max(0.0f, data_count() - 180.0f);
	}

	virtual float min_y() const
	{
		return std::min(0.0f, *std::min_element(mModel.begin(), mModel.end()));
	}

	virtual float max_x() const
	{
		return (float)mModel.size();
	}

	virtual float max_y() const
	{
		return std::max(0.0f, *std::max_element(mModel.begin(), mModel.end()));
	}

	virtual std::size_t data_count() const
	{
		return mModel.size();
	}

	virtual void data_point(int idx, float* x, float* y ) const
	{
		auto it = mModel.begin();
		std::advance(it, idx);
		*x = float(idx);
		*y = *it;
	}
private:
	std::list<float>& mModel;
};


class AiGame: public AbstractGame
{
public:
	AiGame()
	{
		mAdapter.reset(new ChartAdapter(mFitnessData));
		mRenderer.reset(new ChartRenderer(mAdapter.get(), sf::FloatRect()));
		mAiTest.start();
	}

	virtual void move( float dt )
	{
		auto new_stats = mAiTest.get_new_fitness();
		for(auto stat : new_stats)
		{
			std::cout << "Gen " << stat.generation <<  "[" << stat.min_fitness << ", " << stat.avg_fitness << ", " << stat.max_fitness << "]" << std::endl;
			mFitnessData.push_back(stat.avg_fitness);
		}

		while(mFitnessData.size() > 180)
		{
			mFitnessData.pop_front();
		}

		if(!new_stats.empty())
			mRenderer->notifiy_update();
	}

	virtual void render( sf::RenderTarget& target )
	{
		if(mFitnessData.size() > 1)
		{
			mRenderer->render(target);
		}
	}

	virtual void window_resized( sf::Vector2u& size )
	{
	}

	virtual void key_pressed( sf::Keyboard::Key key )
	{
	}

	virtual AbstractGame* next_game()
	{
		return nullptr;
	}

	virtual void close_game()
	{
		mAiTest.stop();
	}

private:
	std::list<float> mFitnessData;
	AiTest mAiTest;

	std::unique_ptr<ChartAdapter> mAdapter;
	std::unique_ptr<ChartRenderer> mRenderer;
};

AbstractGame* GetAiGame()
{
	static AiGame* game = new AiGame();
	return game;
} 