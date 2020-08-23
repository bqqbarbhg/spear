#include "GuiBuilder.h"

namespace cl { namespace gui {

sf_inline uint32_t hashWidget(uint32_t type, uint64_t id)
{
	return sf::hashCombine(sf::hash(id), type);
}

void GuiBuilder::pushBuilderImp(Widget *w)
{
	stackTop++;
	if (stackTop >= (int32_t)stack.size) stack.push();
	WidgetBuilder &b = stack[stackTop];
	b.parent = w;
	b.children.reserveGeometric(w->children.size);
	b.children.size = w->children.size;
	b.nextChildIx = 0;
	sf::Box<Widget> *dst = b.children.data;
	for (sf::Box<Widget> &cw : w->children) {
		new (dst) sf::Box<Widget>(std::move(cw));
		dst++;
	}
	w->children.size = 0;
}

void GuiBuilder::init(const sf::Box<Widget> &root)
{
	this->root = root;
	this->stackTop = -1;
	pushBuilderImp(root);
}

Widget *GuiBuilder::pushImp(uint32_t type, uint64_t id)
{
	WidgetBuilder &b = stack[stackTop];

	// Optimistic: All children in order
	uint32_t nextChild = b.nextChildIx;
	if (nextChild < b.children.size) {
		sf::Box<Widget> &bw = b.children[nextChild];
		if (bw->match(type, id)) {
			b.nextChildIx = nextChild + 1;
			Widget *found = bw;
			b.parent->children.push(std::move(bw));
			found->created = false;
			pushBuilderImp(found);
			return found;
		} else {
			b.nextChildIx = ~0u;

			// Generate mapping of remaining children
			b.childMap.clear();
			b.childMap.reserve(b.children.size - nextChild);
			for (Widget *w : b.children.slice().drop(nextChild)) {
				uint32_t hash = hashWidget(w->type, w->id);
				b.childMap.insertDuplicate(hash, nextChild);
				nextChild++;
			}
		}
	}

	// Optimistic: All children used
	if (b.nextChildIx == b.children.size) return NULL;

	// Search hash map
	uint32_t hash = hashWidget(type, id);
	uint32_t ix = b.childMap.removeOne(hash, ~0u);
	if (ix != ~0u) {
		sf::Box<Widget> &bw = b.children[ix];
		Widget *found = bw;
		b.parent->children.push(std::move(bw));
		found->created = false;
		pushBuilderImp(found);
		return found;
	}

	return nullptr;
}

void GuiBuilder::pop()
{
	sf_assert(stackTop > 0);
	WidgetBuilder &b = stack[stackTop];
	b.parent = nullptr;
	b.nextChildIx = 0;
	b.children.clear();
	b.childMap.clear();
	stackTop--;
}

void GuiBuilder::finish()
{
	sf_assert(stackTop == 0);
	WidgetBuilder &b = stack[stackTop];
	b.parent = nullptr;
	b.nextChildIx = 0;
	b.children.clear();
	b.childMap.clear();
	stackTop--;
}

} }
