#pragma once
#ifndef _CHART_DATA_HPP
#define _CHART_DATA_HPP

#include <cstddef>

class ChartData
{
public:
	virtual float min_x() const = 0;
	virtual float min_y() const = 0;
	virtual float max_x() const = 0;
	virtual float max_y() const = 0;

	virtual std::size_t data_count() const = 0;
	virtual void data_point(int idx, float* x, float* y) const = 0;
};




#endif