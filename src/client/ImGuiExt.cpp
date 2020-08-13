#include "ImGuiExt.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ext/imgui/imgui_internal.h"

using namespace ImGui;

namespace cl {

ImEditState ImBSplineEditor(const char *label, ImVec2 editorSize, float *values, int *p_numPoints, int maxPoints)
{
	ImVec2 *points = (ImVec2*)values;

	if (editorSize.x <= 0.0f) editorSize.x = CalcItemWidth();

	const ImGuiStyle& Style = GetStyle();
	const ImGuiIO& IO = GetIO();
	ImDrawList* DrawList = GetWindowDrawList();
	ImGuiWindow* Window = GetCurrentWindow();
	ImRect bb { Window->DC.CursorPos, Window->DC.CursorPos + editorSize };
	ItemSize(bb);
	if (!ItemAdd(bb, NULL)) return ImEditState::None;

	const ImGuiID id = Window->GetID(label);
	static const ImGuiID ID_DragPointX = 1000;
	static const ImGuiID ID_DragPointY = 1001;

	bool areaHovered = IsItemHovered();
	bool areaActive = GetActiveID() == id;

	RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, Style.FrameRounding);

	ImVec2 relBase = ImVec2(bb.Min.x + 5.0f, bb.Max.y - 5.0f);
	ImVec2 relScale = ImVec2(bb.Max.x - bb.Min.x - 10.0f, bb.Min.y - bb.Max.y + 10.0f);

	ImU32 baseCol = ImColor(Style.Colors[ImGuiCol_Button]);
	ImU32 hoverCol = ImColor(Style.Colors[ImGuiCol_ButtonHovered]);
	ImU32 activeCol = ImColor(Style.Colors[ImGuiCol_ButtonActive]);

	float dragRadius = 8.0f;
	int closestPoint = -1;
	float closestDistSq = dragRadius * dragRadius;

	int numPoints = *p_numPoints;
	bool dragging = false;
	bool changed = false;
	ImEditState state = ImEditState::None;

	float prevX = Window->StateStorage.GetFloat(ID_DragPointX, -1.0f);
	float prevY = Window->StateStorage.GetFloat(ID_DragPointY, -1.0f);
	if (areaHovered || areaActive) {

		for (int i = 0; i < numPoints; i++) {
			ImVec2 &point = points[i];
			ImVec2 relPoint = point * relScale + relBase;

			float dist = ImLengthSqr(relPoint - IO.MousePos);

			if (point.x == prevX && point.y == prevY) {
				dist = 0.0f;
			}

			if (dist < closestDistSq) {
				closestPoint = i;
				closestDistSq = dist;
			}
		}

		if (closestPoint >= 0 && (ImGui::IsMouseDown(0))) {
			ImVec2 &point = points[closestPoint];
			point += IO.MouseDelta / relScale;
			point = ImClamp(point, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
			ClearActiveID();
			SetActiveID(id, Window);
			FocusWindow(Window);

			Window->StateStorage.SetFloat(ID_DragPointX, point.x);
			Window->StateStorage.SetFloat(ID_DragPointY, point.y);

			dragging = true;
			changed = true;
			state = ImEditState::Dragging;
		}

		if (ImGui::IsMouseDoubleClicked(0)) {
			ClearActiveID();
			SetActiveID(id, Window);
			FocusWindow(Window);

			if (closestPoint >= 0) {
				points[closestPoint] = points[numPoints - 1];
				numPoints--;
				--*p_numPoints;
			} else {
				if (numPoints < maxPoints) {
					ImVec2 point = (IO.MousePos - relBase) / relScale;
					point = ImClamp(point, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
					points[numPoints] = point;
					numPoints++;
					++*p_numPoints;
				}
			}
			changed = true;
			state = ImEditState::Changed;
		}
	}

	if (!ImGui::IsMouseDown(0)) {
		if (areaActive) {
			ClearActiveID();
		}
		if (prevX >= 0.0f) { state = ImEditState::Changed; Window->StateStorage.SetFloat(ID_DragPointX, -1.0f); }
		if (prevY >= 0.0f) { state = ImEditState::Changed; Window->StateStorage.SetFloat(ID_DragPointY, -1.0f); }
	}

	for (int i = 0; i < numPoints; i++) {
		ImVec2 &point = points[i];
		ImVec2 relPoint = point * relScale + relBase;

		ImU32 color = baseCol;
		if (closestPoint == i) color = dragging ? activeCol : hoverCol;

		DrawList->AddCircle(relPoint, 5.0f, color);
	}

	if (changed) {
		qsort(points, numPoints, sizeof(ImVec2), [](const void *va, const void *vb) {
			const ImVec2 &a = *(const ImVec2*)va, &b = *(const ImVec2*)vb;
			if (a.x < b.x) return -1;
			if (a.x > b.x) return +1;
			return 0;
		});
	}

	if (numPoints > 0) {
		ImU32 plotCol = GetColorU32(ImGuiCol_PlotLines);

		bool hasP0 = false;
		ImVec2 p0;

		if (numPoints >= 0) {
			ImVec2 a = relBase + relScale * ImVec2(0.f, points[0].y);
			ImVec2 b = relBase + relScale * ImVec2(points[0].x, points[0].y);
			ImVec2 c = relBase + relScale * ImVec2(points[numPoints - 1].x, points[numPoints - 1].y);
			ImVec2 d = relBase + relScale * ImVec2(1.0f, points[numPoints - 1].y);
			DrawList->AddLine(a, b, plotCol);
			DrawList->AddLine(c, d, plotCol);
		}

		for (int i = -1; i < numPoints; i++) {
			ImVec2 a = points[i - 1 >= 0 ? i - 1 : 0];
			ImVec2 b = points[i >= 0 ? i : 0];
			ImVec2 c = points[i + 1 < numPoints ? i + 1 : numPoints - 1];
			ImVec2 d = points[i + 2 < numPoints ? i + 2 : numPoints - 1];

			int steps = 100;
			for (int s = 0; s < steps; s++) {
				float t = (float)s / (float)(steps - 1);
				float nt = 1.0f - t;
				float t2 = t*t;
				float t3 = t2*t;
				float nt2 = nt*nt;
				float nt3 = nt2*nt;

				ImVec2 p1;
				p1 += a * (nt3 / 6.0f);
				p1 += b * ((3.0f*t3 - 6.0f*t2 + 4.0f) / 6.0f);
				p1 += c * ((-3.0f*t3 + 3.0f*t2 + 3.0f*t + 1.0f) / 6.0f);
				p1 += d * (t3 / 6.0f);
				
				p1 = p1 * relScale + relBase;

				float dist = ImLengthSqr(p0 - p1);
				const float minDist = 3.0f;
				if (hasP0 && (dist > minDist * minDist || (s == steps - 1 && i == numPoints - 1))) {
					DrawList->AddLine(p0, p1, plotCol);
					p0 = p1;
				}

				if (!hasP0) {
					p0 = p1;
					hasP0 = true;
				}
			}
		}
	}

	return state;
}

ImEditState ImGradientEditor(const char *label, ImVec2 editorSize, float *values, int *p_numPoints, int maxPoints, int *p_selectedIndex, const float *defaultColor)
{
	ImVec4 *points = (ImVec4*)values;

	if (editorSize.x <= 0.0f) editorSize.x = CalcItemWidth();

	const ImGuiStyle& Style = GetStyle();
	const ImGuiIO& IO = GetIO();
	ImDrawList* DrawList = GetWindowDrawList();
	ImGuiWindow* Window = GetCurrentWindow();
	ImRect bb { Window->DC.CursorPos, Window->DC.CursorPos + editorSize };
	ItemSize(bb);
	if (!ItemAdd(bb, NULL)) return ImEditState::None;

	const ImGuiID id = Window->GetID(label);
	static const ImGuiID ID_DragPointX = 2000;
	static const ImGuiID ID_SelectedPointX = 2001;

	ImEditState state = ImEditState::None;
	bool changed = false;
	bool dragging = false;
	int numPoints = *p_numPoints;

	bool areaHovered = IsItemHovered();
	bool areaActive = GetActiveID() == id;

	float height = editorSize.y - 2.0f;
	int closestPoint = -1;
	float closestDist = height * 0.5f;

	float width = 2.0f;

	float relBase = bb.Min.x + width * 0.5f;
	float relScale = bb.Max.x - bb.Min.x - width;

	// RenderFrame(bb.Min, bb.Max, 0xff000000, true, Style.FrameRounding);

	float prevX = Window->StateStorage.GetFloat(ID_DragPointX, -1.0f);

	if (areaHovered || areaActive) {

		if (IO.MousePos.y >= bb.Min.y && IO.MousePos.y <= bb.Max.y) {
			for (int i = 0; i < numPoints; i++) {
				ImVec4 &point = points[i];
				float relPointX = point.x * relScale + relBase;

				float dist = ImFabs(relPointX - IO.MousePos.x);

				if (point.x == prevX) {
					dist = 0.0f;
				}

				if (dist <= closestDist) {
					closestPoint = i;
					closestDist = dist;
				}
			}
		}

		if (closestPoint >= 0 && (ImGui::IsMouseDown(0))) {
			ImVec4 &point = points[closestPoint];
			point.x += IO.MouseDelta.x / relScale;
			point.x = ImClamp(point.x, 0.0f, 1.0f);
			ClearActiveID();
			SetActiveID(id, Window);
			FocusWindow(Window);

			Window->StateStorage.SetFloat(ID_DragPointX, point.x);
			Window->StateStorage.SetFloat(ID_SelectedPointX, point.x);

			dragging = true;
			changed = true;
			state = ImEditState::Dragging;
		}

		if (ImGui::IsMouseDoubleClicked(0)) {
			ClearActiveID();
			SetActiveID(id, Window);
			FocusWindow(Window);

			if (closestPoint >= 0) {
				points[closestPoint] = points[numPoints - 1];
				numPoints--;
				--*p_numPoints;
			} else {
				if (numPoints < maxPoints) {
					ImVec4 point;

					if (numPoints > 0) {
						point = points[0];
					} else {
						point = ImVec4(1.0f, defaultColor[0], defaultColor[1], defaultColor[2]);
					}

					float t = ImClamp((IO.MousePos.x - relBase) / relScale, 0.0f, 1.0f);
					for (int i = 0; i < numPoints; i++) {
						if (points[i].x < t) {
							if (i + 1 < numPoints) {
								float minT = points[i].x;
								float maxT = points[i + 1].x;

								float relT = (t - minT) / (maxT - minT);
								point = ImLerp(points[i], points[i + 1], relT);

							} else {
								point = points[i];
							}
							break;
						}
					}

					point.x = t;

					points[numPoints] = point;
					numPoints++;
					++*p_numPoints;
				}
			}
			changed = true;
			state = ImEditState::Changed;
		}
	}

	if (!ImGui::IsMouseDown(0)) {
		if (areaActive) {
			ClearActiveID();
		}
		if (prevX >= 0.0f) { state = ImEditState::Changed; Window->StateStorage.SetFloat(ID_DragPointX, -1.0f); }
	}

	if (changed) {
		qsort(points, numPoints, sizeof(ImVec4), [](const void *va, const void *vb) {
			const ImVec4 &a = *(const ImVec4*)va, &b = *(const ImVec4*)vb;
			if (a.x < b.x) return -1;
			if (a.x > b.x) return +1;
			return 0;
		});
	}

	int selectedIndex = -1;
	float selectedX = Window->StateStorage.GetFloat(ID_SelectedPointX, -1.0f);
	if (selectedX >= 0.0f) {
		for (int i = 0; i < numPoints; i++) {
			if (points[i].x == selectedX) {
				selectedIndex = i;
				break;
			}
		}
	}

	const float rounding = 2.0f;
	const float thickness = 2.0f;
	float mid = (bb.Min.y + bb.Max.y) * 0.5f;

	for (int i = -1; i < numPoints; i++) {
		ImVec4 pt0, pt1;
		if (numPoints > 0) {
			if (i >= 0) {
				pt0 = points[i];
			} else {
				pt0 = points[i + 1];
				pt0.x = 0.0f;
			}
			if (i + 1 < numPoints) {
				pt1 = points[i + 1];
			} else {
				pt1 = points[i];
				pt1.x = 1.0f;
			}
		} else {
			pt0 = ImVec4(0.0f, defaultColor[0], defaultColor[1], defaultColor[2]);
			pt1 = ImVec4(1.0f, defaultColor[0], defaultColor[1], defaultColor[2]);
		}

		float x0 = pt0.x * relScale + relBase;
		float x1 = pt1.x * relScale + relBase;

		ImVec2 min = ImVec2(x0, bb.Min.y);
		ImVec2 max = ImVec2(x1, bb.Max.y);

		ImU32 col0 = ImColor(pt0.y, pt0.z, pt0.w);
		ImU32 col1 = ImColor(pt1.y, pt1.z, pt1.w);

		DrawList->AddRectFilledMultiColor(min, max, col0, col1, col1, col0);
	}

	DrawList->AddRect(bb.Min, bb.Max, 0xff888888, Style.FrameRounding);

	for (int i = 0; i < numPoints; i++) {
		const ImVec4 &point = points[i];

		float x = point.x * relScale + relBase;
		ImVec2 min = ImVec2(x - 1.0f * 0.5f, mid - height * 0.5f);
		ImVec2 max = ImVec2(x + 1.0f, mid + height * 0.5f);

		DrawList->AddRectFilled(min, max, ImColor(point.y, point.z, point.w), rounding);

		uint32_t borderColor = 0xff888888;
		if (i == selectedIndex) {
			borderColor = 0xffffffff;
		}
		DrawList->AddRect(min - ImVec2(thickness, thickness), max + ImVec2(thickness, thickness),
			borderColor, rounding, 15, thickness);
	}

	*p_selectedIndex = selectedIndex;

	return state;
}

}

