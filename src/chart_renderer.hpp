#pragma once
#ifndef _CHART_RENDERER_HPP
#define _CHART_RENDERER_HPP


#include <vector>
#include <memory>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include "chart_data.hpp"


class ChartRenderer
{
public:
	ChartRenderer(const ChartData* data, const sf::FloatRect& render_rect);
	~ChartRenderer();

	void show_axes(bool show);
	bool show_axes();

	void render_rect(const sf::FloatRect& render_rect);
	const sf::FloatRect& render_rect();

	void render(sf::RenderTarget& target);
	void notifiy_update();

private:
	void update();

private:
	bool mHasUpdate;
	bool mShowAxes;
	sf::FloatRect mRenderRect;
	const ChartData* mData;

	sf::View mView;
	std::vector<sf::Vertex> mVertexList;
};


#endif