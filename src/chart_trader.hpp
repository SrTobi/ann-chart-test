#pragma once
#ifndef _CHART_TRADER_HPP
#define _CHART_TRADER_HPP

#include <functional>

class WalkingChart;
class ChartTrader;

typedef std::function<float(float)> ChargeFunction;

class Order
{
	friend class ChartTrader;
public:
	enum OrderType
	{
		Short,
		Long
	};
public:
	~Order();

	OrderType type() const;
	bool active() const;

	void breach();
	float leave();

	float entrance() const;

private:
	Order(OrderType type, ChartTrader* trader, ChargeFunction charge_function);

private:
	const OrderType mOrderType;
	ChartTrader* mTrader;
	bool mActive;
	float mEntrance;
	const ChargeFunction mChargeFunction;;
};



class ChartTrader
{
	friend class Order;
public:
	ChartTrader(WalkingChart* chart, float capital, ChargeFunction charge_function = [](float){return 0.0f;});
	~ChartTrader();

	float capital() const;
	Order& short_order();
	Order& long_order();
	const Order& short_order() const;
	const Order& long_order() const;

	bool is_trading() const;

private:
	WalkingChart* mChart;
	float mCapital;
	Order mShortOrder;
	Order mLongOrder;
};



#endif