#pragma once

#include "sf/Base.h"
#include "sf/Mutex.h"
#include "sf/String.h"

namespace sp {

struct Asset;

struct AssetType
{
	typedef void (*InitFnImp)(Asset *);

	const char *name;
	size_t size;
	InitFnImp init;
};

struct Asset
{
	sf::StringBuf name;
	const AssetType *type;
	uint32_t state;
	uint32_t refcount;
	uint32_t flags;

	Asset();
	virtual ~Asset();

	bool isLoaded() const;
	bool shouldBeLoaded();

	void startLoading();

	void retain();
	void release();

	static Asset *findImp(const sf::String &name, const AssetType *type);
	static Asset *createImp(const sf::String &name, const AssetType *type);

	template <typename T>
	static T *find(const sf::String &name) {
		return (T*)findImp(name, &T::AssetType);
	}

	template <typename T>
	static T *load(const sf::String &name) {
		return (T*)createImp(name, &T::AssetType);
	}

	// Virtuals
	virtual void startLoadingImp() = 0;

	// Implementation
	void finishLoadingImp();
	void freeImp();
};

template <typename T>
struct Ref
{
	T *ptr;
	Ref() : ptr(nullptr) { }
	explicit Ref(const sf::String &name) { ptr = Asset::load<T>(name); }
	explicit Ref(T *t) : ptr(t) { }
	Ref(const Ref &r) : ptr(r.ptr) { ptr->retain(); }
	Ref(Ref &&r) : ptr(r.ptr) { r.ptr = nullptr; }
	~Ref() { if (ptr) ptr->release(); }

	Ref &operator=(const Ref &r) {
		if (&r == this) return;
		if (ptr) ptr->release();
		ptr = r->ptr;
		ptr->retain();
		return *this;
	}

	Ref &operator=(Ref &&r) {
		if (&r == this) return;
		ptr = r->ptr;
		r->ptr = nullptr;
		return *this;
	}

	T *operator->() { return ptr; }
};

}