
#include "CardComponent.h"
#include "sp/Model.h"
#include "game/system/ModelSystem.h"

struct CardModel final : CardComponent
{
	sf::StringBuf bone;
	sp::ModelRef model;

	virtual void render() override
	{
		sf::Mat34 transform = getModelBoneTransform(owner, bone);
		renderModel(model, transform);
	}
};
