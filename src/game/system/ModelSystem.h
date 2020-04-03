#pragma once

#include "game/Entity.h"
#include "sf/Matrix.h"
#include "sf/String.h"
#include "sp/Model.h"

sf::Mat34 getModelBoneTransform(Entity e, sf::String boneName);

void renderModel(sp::Model *model, const sf::Mat34 &transform);
