#pragma once

#include "game/Entity.h"
#include "game/Map.h"
#include "game/render/MapRenderer.h"

struct GameCamera
{
	sf::Vec3 position;
	sf::Mat34 worldToView;
	sf::Mat44 viewToClip;
	sf::Mat44 worldToClip;
};

struct Game
{
	GameCamera camera;
	Map map;
	MapRenderer mapRenderer;
};

extern thread_local Game *t_game;

