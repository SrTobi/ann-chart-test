#include <cassert>
#include "chart_trader.hpp"
#include "realtime_chart.hpp"


Order::~Order()
{
}

Order::OrderType Order::type() const
{
	return mOrderType;
}

bool Order::active() const
{
	return mActive;
}

void Order::breach()
{
	assert(!mActive);
	mEntrance = mTrader->mChart->current_value();

	assert(mEntrance >= 0.0f);

	//mTrader->mCapital -= mEntrance;
	mTrader->mCapital -= mChargeFunction(mEntrance);
	mActive = true;
}

float Order::leave()
{
	assert(mActive);

	float& capital = mTrader->mCapital;
	float old_capital = capital;
	float cur_value = mTrader->mChart->current_value();
	float diff = cur_value - mEntrance;

	// charges
	capital -= mChargeFunction(cur_value);

	switch (mOrderType)
	{
	case Long:
		capital += diff;
		break;
	case Short:
		capital -= diff;
		break;
	default:
		throw std::exception("Unknown order type");
	}

	mActive = false;
	return capital - old_capital;
}

Order::Order( OrderType type, ChartTrader* trader, ChargeFunction charge_function )
	: mOrderType(type)
	, mTrader(trader)
	, mChargeFunction(charge_function)
	, mEntrance(0.0f)
	, mActive(false)
{
}

float Order::entrance() const
{
	assert(active());
	return mEntrance;
}


ChartTrader::ChartTrader( WalkingChart* chart, float capital, ChargeFunction charge_function)
	: mChart(chart)
	, mCapital(capital)
	, mLongOrder(Order::Long, this, charge_function)
	, mShortOrder(Order::Short, this, charge_function)
{

}

ChartTrader::~ChartTrader()
{
}

float ChartTrader::capital() const
{
	return mCapital;
}

Order& ChartTrader::short_order()
{
	return mShortOrder;
}

Order& ChartTrader::long_order()
{
	return mLongOrder;
}

const Order& ChartTrader::short_order() const
{
	return mShortOrder;
}

const Order& ChartTrader::long_order() const
{
	return mLongOrder;
}

bool ChartTrader::is_trading() const
{
	return long_order().active() || short_order().active();
}
