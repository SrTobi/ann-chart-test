#include "chart_renderer.hpp"





ChartRenderer::ChartRenderer( const ChartData* data, const sf::FloatRect& render_rect )
	: mData(data)
	, mRenderRect(render_rect)
	, mHasUpdate(true)
	, mShowAxes(true)
{

}

ChartRenderer::~ChartRenderer()
{

}

void ChartRenderer::show_axes( bool show )
{
	mShowAxes = show;
}

bool ChartRenderer::show_axes()
{
	return mShowAxes;
}

void ChartRenderer::render_rect( const sf::FloatRect& render_rect )
{
	mRenderRect = render_rect;
}

const sf::FloatRect& ChartRenderer::render_rect()
{
	return mRenderRect;
}

void ChartRenderer::render(sf::RenderTarget& target)
{
	if(mHasUpdate)
		update();


	sf::View oldView = target.getView();
	{
		target.setView(mView);
		target.draw(mVertexList.data(), mVertexList.size(), sf::LinesStrip);

		sf::Vertex line[] =
		{
			sf::Vertex(sf::Vector2f(mView.getCenter().x - mView.getSize().x, mView.getCenter().y * 2)),
			sf::Vertex(sf::Vector2f(mView.getCenter().x + mView.getSize().x, mView.getCenter().y * 2))
		};

		target.draw(line, 2, sf::LinesStrip);
	}
	target.setView(oldView);
}

void ChartRenderer::notifiy_update()
{
	mHasUpdate = true;
}

void ChartRenderer::update()
{
	mVertexList.clear();

	unsigned int count = mData->data_count();

	sf::Vector2f size(mData->max_x() - mData->min_x(), mData->max_y() - mData->min_y());
	sf::Vector2f center(mData->min_x() + size.x * 0.5f,
						mData->min_y() + size.y * 0.5f);


	float top = mData->max_y();
	mView = sf::View(center, size);

	for(std::size_t idx = 0; idx < count; ++idx)
	{
		float x,y;
		mData->data_point(idx, &x, &y);

		mVertexList.emplace_back(sf::Vector2f(x, center.y * 2 - y));
	}
	mHasUpdate = false;
}
