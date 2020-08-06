#pragma once

#include "sf/Matrix.h"

namespace cl {

struct EntityState
{
	enum Flags {
		Visible = 0x1,
	};

	enum UpdateMask {
		UpdateTransform = 0x1,
		UpdateVisibility = 0x2,
	};

	sf::Mat34 transform;
	uint32_t flags;
};

}
