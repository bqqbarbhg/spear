#include "Arena.h"

namespace sf {

static constexpr size_t Alignment = 64;

Arena::Arena(Arena&& a)
{
	page = a.page;
	pagePos = a.pagePos,
	pageSize = a.pageSize;
	growSize = a.growSize;
	oldPagesTotalSize = a.oldPagesTotalSize;
	oldPagesSlack = a.oldPagesSlack;
	a.page = nullptr;
	a.pagePos = 0;
	a.pageSize = 0;
	a.growSize = InitialPageSize;
	a.oldPagesTotalSize = 0;
	a.oldPagesSlack = 0;
}

Arena& Arena::operator=(Arena&& a)
{
	// TODO: Factor to clear()
	{
		void *toFree = page;
		page = nullptr;
		while (toFree) {
			void *next = *(void**)toFree;
			alignedFree(toFree);
			toFree = next;
		}
	}

	page = a.page;
	pagePos = a.pagePos,
	pageSize = a.pageSize;
	growSize = a.growSize;
	oldPagesTotalSize = a.oldPagesTotalSize;
	oldPagesSlack = a.oldPagesSlack;
	a.page = nullptr;
	a.pagePos = 0;
	a.pageSize = 0;
	a.growSize = InitialPageSize;
	a.oldPagesTotalSize = 0;
	a.oldPagesSlack = 0;
	return *this;
}

Arena::~Arena()
{
	void *toFree = page;
	page = nullptr;
	while (toFree) {
		void *next = *(void**)toFree;
		alignedFree(toFree);
		toFree = next;
	}
}

void *Arena::pushPageImp(size_t minSize, size_t align)
{
	oldPagesSlack += pageSize - pagePos;
	oldPagesTotalSize += pageSize;

	size_t size = max(growSize, minSize + align);
	growSize *= 2;
	void *newPage = alignedAlloc(size, Alignment);

	size_t pos = pagePos + ((size_t)-(intptr_t)sizeof(void*) & align);
	*(void**)newPage = page;

	page = (char*)newPage;
	pagePos = pos + minSize;
	pageSize = size;

	return (char*)newPage + pos;
}

}