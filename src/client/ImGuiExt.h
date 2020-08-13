#pragma once

#include "ext/imgui/imgui.h"

namespace cl {

enum class ImEditState
{
	None,
	Dragging,
	Changed,
};

ImEditState ImBSplineEditor(const char *label, ImVec2 editorSize, float *values, int *p_numPoints, int maxPoints);
ImEditState ImGradientEditor(const char *label, ImVec2 editorSize, float *values, int *p_numPoints, int maxPoints, int *p_selectedIndex, const float *defaultColor);

}
