#include "CardModel.h"
#include "Entity.h"

#include "sp/Model.h"

struct CardModel
{
	Entity owner;
	sp::ModelRef model;
	sf::StringBuf bone;
};

struct CardModelComponent : Component
{

};

Component *getCardModel()
{
	static CardModelComponent c;
	return &c;
}

