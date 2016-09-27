#include <chrono>
#include <cassert>
#include "chart_model.hpp"


ChartModel::ChartModel(float min_vlaue, float max_value, float volatility, std::size_t tick_count)
	: mMinValue(min_vlaue)
	, mMaxValue(max_value)
	, mVolatility(volatility)
	, mChartValues(tick_count, 0.0f)
{
	generate();
}

ChartModel::~ChartModel()
{

}

float ChartModel::min_value() const
{
	return mMinValue;
}

float ChartModel::max_value() const
{
	return mMaxValue;
}

float ChartModel::value(std::size_t tick ) const
{
	return mChartValues.at(tick);
}

std::size_t ChartModel::tick_count() const
{
	return mChartValues.size();
}

void ChartModel::generate()
{
	generate((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
}

void ChartModel::generate(unsigned int seed)
{
	const float abs_vol = mVolatility * (max_value() - min_value());
	typedef std::uniform_real_distribution<float> distribution_type;
	std::default_random_engine generator(seed);
	distribution_type absolute_distribution(min_value(), max_value());
	distribution_type delta_distribution(min_value() / 3.0f, max_value() / 3.0f);

	auto abs_rand = [&]() { return absolute_distribution(generator); };
	auto rel_rand = [&]() { return delta_distribution(generator); };

	float cur_value = abs_rand();

	for(auto& value : mChartValues)
	{
		value = cur_value;

		float min = std::max(-abs_vol/ 2.0f, min_value() - cur_value);
		float max = std::min(abs_vol/2.0f, max_value() - cur_value);
		assert(cur_value + min >= min_value());
		assert(cur_value + max <= max_value());

		delta_distribution = distribution_type(min, max);
		cur_value += rel_rand();
	}
}
