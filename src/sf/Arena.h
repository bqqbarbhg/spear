#pragma once

#include "sf/Base.h"

namespace sf {

struct Arena
{
	static constexpr size_t InitialPageSize = 4096;

	char *page = nullptr;
	size_t pagePos = 0, pageSize = 0;
	size_t growSize = InitialPageSize;
	size_t oldPagesTotalSize = 0;
	size_t oldPagesSlack = 0;

	Arena() { }
	Arena(const Arena&) = delete;
	Arena& operator=(const Arena&) = delete;
	Arena(Arena&& a);
	Arena& operator=(Arena&& a);
	~Arena();

	size_t usedSize() const { return oldPagesTotalSize + pagePos - oldPagesSlack; }
	size_t allocatedSize() const { return oldPagesTotalSize + pageSize; }

	void *pushPageImp(size_t minSize, size_t align);

	void *pushSizeUninit(size_t size, size_t align) {
		size_t pos = pagePos + ((size_t)-(intptr_t)pagePos & align);
		if (pos + size < pageSize) {
			pagePos = pos + size;
			return page + pos;
		} else {
			return pushPageImp(size, align);
		}
	}

	template <typename T>
	T *push() {
		T *t = (T*)pushSizeUninit(sizeof(T), alignof(T));
		new (t) T();
		return t;
	}

	template <typename T>
	T *pushPtr(size_t num) {
		T *t = (T*)pushSizeUninit(num * sizeof(T), alignof(T));
		constructRangeImp<T>(t, num);
		return slice(t, num);
	}

	template <typename T>
	Slice<T> pushSlice(size_t num) {
		T *t = (T*)pushSizeUninit(num * sizeof(T), alignof(T));
		constructRangeImp<T>(t, num);
		return Slice<T>(t, num);
	}

	template <typename T>
	T *pushUninit() {
		T *t = (T*)pushSizeUninit(sizeof(T), alignof(T));
		return t;
	}

	template <typename T>
	T* pushPtrUninit(size_t num) {
		T *t = (T*)pushSizeUninit(num * sizeof(T), alignof(T));
		return t;
	}

	template <typename T>
	Slice<T> pushSliceUninit(size_t num) {
		T *t = (T*)pushSizeUninit(num * sizeof(T), alignof(T));
		return Slice<T>(t, num);
	}

};

}

