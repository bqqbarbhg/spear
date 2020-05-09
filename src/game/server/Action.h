#pragma once

namespace sv {

struct Action
{
	enum Type
	{
		Error,
		Move,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};
};

}
