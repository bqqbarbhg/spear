#pragma once

#include "sf/Box.h"
#include "sf/HashMap.h"
#include "sf/UintMap.h"
#include "client/gui/Gui.h"

namespace cl { namespace gui {

struct GuiBuilder
{
	struct WidgetBuilder
	{
		Widget *parent = nullptr;
		uint32_t nextChildIx = 0;
		sf::Array<sf::Box<Widget>> children;
		sf::UintMap childMap;
	};

	sf::Box<Widget> root;
	sf::Array<WidgetBuilder> stack;
	int32_t stackTop = -1;

	void pushBuilderImp(Widget *w);

	void init(const sf::Box<Widget> &root);
	Widget *pushImp(uint32_t type, uint64_t id);

	template <typename T>
	T *push(uint64_t id)
	{
		const uint32_t type = T::WidgetType;
		T *found = (T*)pushImp(type, id);
		if (found) return found;

		WidgetBuilder &b = stack[stackTop];

		sf::Box<Widget> bw = sf::box<T>();
		Widget *w = bw;
		b.parent->children.push(std::move(bw));
		w->id = id;
		pushBuilderImp(w);
		return (T*)w;
	}

	template <typename T>
	T *push(const void *id)
	{
        return push<T>((uint64_t)(uintptr_t)id);
	}

	template <typename T>
	T *push()
	{
		return push<T>(NoWidgetId);
	}

	void pop();

	void finish();
};

} }
