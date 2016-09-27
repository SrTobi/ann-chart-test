#pragma once
#ifndef _REALTIME_CHART_HPP
#define _REALTIME_CHART_HPP


#include "chart_model.hpp"


class WalkingChart
{
public:
	WalkingChart(ChartModel* back_model);
	virtual ~WalkingChart();
	virtual std::size_t current_tick() const = 0;

	float min_value() const;
	float max_value() const;

	float current_value() const;
	std::size_t data_count() const;
	float tick_value(std::size_t tick) const;

	std::size_t max_ticks() const;
	bool is_done() const;
private:
	const ChartModel* mBackModel;
};

class RealtimeChart: public WalkingChart
{
public:
	RealtimeChart(ChartModel* back_model, float ticks_per_second);
	~RealtimeChart();

	virtual std::size_t current_tick() const;

	bool walk_time(float time);

	float current_time() const;
	float time_value(float time) const;
	float ticks_per_second() const;
private:
	const float mTicksPerSecond;
	float mCurrentTime;
};

class TickChart: public WalkingChart
{
public:
	TickChart(ChartModel* back_model);
	~TickChart();

	bool walk_tick(std::size_t count = 1);
	virtual std::size_t current_tick() const;

private:
	std::size_t mCurrentTick;
};

#endif