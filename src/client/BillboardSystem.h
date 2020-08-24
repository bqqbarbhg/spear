#pragma once

#include "client/System.h"

namespace sp { struct Sprite; }

namespace cl {

struct Billboard
{
	sp::Sprite *sprite = nullptr;
	sf::Mat34 transform;
	sf::Vec4 color = sf::Vec4(1.0f);
	float depth = 0.0f;
	sf::Vec2 anchor = sf::Vec2(0.5f);
	sf::Vec2 cropMin = sf::Vec2(0.0f);
	sf::Vec2 cropMax = sf::Vec2(1.0f);
};

struct BillboardSystem : System
{
	static sf::Box<BillboardSystem> create();

	virtual void addBillboard(sp::Sprite *sprite, const sf::Mat34 &transform, const sf::Vec4 &color=sf::Vec4(1.0f), float depth = 0.0f) = 0;
	virtual void addBillboard(const Billboard &billboard) = 0;

	virtual void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;

};

}

