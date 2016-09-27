#include <cassert>
#include <algorithm>
#include "realtime_chart.hpp"



WalkingChart::WalkingChart( ChartModel* back_model)
	: mBackModel(back_model)
{
	assert(mBackModel);
	assert(mBackModel->tick_count() > 0);
}

WalkingChart::~WalkingChart()
{
}


float WalkingChart::current_value() const
{
	return mBackModel->value(data_count() - 1);
}

std::size_t WalkingChart::data_count() const
{
	return std::min(current_tick() + 1, max_ticks());
}

float WalkingChart::tick_value( std::size_t tick ) const
{
	return mBackModel->value(std::min(tick, data_count() - 1));
}


std::size_t WalkingChart::max_ticks() const
{
	return mBackModel->tick_count();
}

bool WalkingChart::is_done() const
{
	return current_tick() >= max_ticks();
}

float WalkingChart::min_value() const
{
	return mBackModel->min_value();
}

float WalkingChart::max_value() const
{
	return mBackModel->max_value();
}


RealtimeChart::RealtimeChart( ChartModel* back_model, float ticks_per_second )
	: WalkingChart(back_model)
	, mTicksPerSecond(ticks_per_second)
	, mCurrentTime(0)
{
	assert(mTicksPerSecond > 0.0f);
}

RealtimeChart::~RealtimeChart()
{

}

std::size_t RealtimeChart::current_tick() const
{
	return std::size_t(mCurrentTime * ticks_per_second());
}

bool RealtimeChart::walk_time( float time )
{
	auto old_ticks = current_tick();
	mCurrentTime += time;

	return old_ticks < current_tick();
}

float RealtimeChart::current_time() const
{
	return mCurrentTime;
}

float RealtimeChart::time_value( float time ) const
{
	return tick_value(std::size_t(time * ticks_per_second()));
}

float RealtimeChart::ticks_per_second() const
{
	return mTicksPerSecond;
}







TickChart::TickChart( ChartModel* back_model )
	: WalkingChart(back_model)
	, mCurrentTick(0)
{
}

TickChart::~TickChart()
{
}

bool TickChart::walk_tick( std::size_t count /*= 1*/ )
{
	mCurrentTick += count;
	return count > 0;
}

std::size_t TickChart::current_tick() const
{
	return mCurrentTick;
}
