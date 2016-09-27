#ifndef _CHART_MODEL_HPP
#define _CHART_MODEL_HPP

#include <vector>
#include <random>
#include <memory>
#include "chart_data.hpp"

class ChartModel
{
public:
	ChartModel(float min_vlaue, float max_value, float volatility, std::size_t tick_count);
	~ChartModel();

	float min_value() const;
	float max_value() const;

	float value(std::size_t tick) const;
	std::size_t tick_count() const;

	void generate();
	void generate(unsigned int seed);

	//float max_yield() const;

private:
	void calc_max_yield();
private:
	std::unique_ptr<float> mMaxYield;
	std::vector<float> mChartValues;
	float mVolatility;
	float mMinValue;
	float mMaxValue;
};





#endif