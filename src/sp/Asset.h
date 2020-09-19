#pragma once

#include "sf/Base.h"
#include "sf/String.h"
#include "sf/Symbol.h"

namespace sp {

struct Asset;

struct AssetType
{
	typedef void (*CtorFn)(Asset *);

	const char *name;    // < Name of the asset type
	size_t instanceSize; // < Size of the asset instances in bytes
	size_t propertySize; // < Size of the asset property struct
	CtorFn ctorFn;       // < Call the instance constructor with properties

	// Implementation
	alignas(void*) char impData[64];
};

// Base class for asset properties
struct AssetProps
{
	virtual ~AssetProps();
	virtual uint32_t hash() const = 0;
	virtual bool equal(const AssetProps &rhs) const = 0;
	virtual void copyTo(AssetProps *uninitDst) const = 0;
};

// Use this for assets that have no properties
struct NoAssetProps : AssetProps
{
	virtual uint32_t hash() const final;
	virtual bool equal(const AssetProps &rhs) const final;
	virtual void copyTo(AssetProps *uninitDst) const;
};


// Asset that can be loaded/derived from from a name and an optional property blob.
// To implement a new asset type you need to override `assetStartLoading() and `assetUnload()`,
// you must also provide `AssetType MyAsset::SelfType` and `typename MyAsset::PropType`.
struct Asset
{
	Asset();
	virtual ~Asset();

	// -- Fields

	AssetType *type;         // < Type of the asset
	sf::Symbol name;         // < Null-terminated name of the asset
	const AssetProps *props; // < Properties of the asset
	uint32_t refcount;       // < Number of references to the asset, do not modify!

	// -- Instance API

	// Manual refcounting
	void retain();
	void release();

	// Asset state query
	bool isLoaded() const;
	bool isFailed() const;

	// Returns `true` if the asset is loaded. Otherwise issues
	// a warning and starts loading the asset returning `false`.
	bool shouldBeLoaded();

	// Request the asset to start loading
	void startLoading();

	// Request the asset to start reloading
	void startReloading();

	// -- API for Asset implementations

	// Start loading the asset asynchronously, call
	// `assetFinishLoading()` or `assetFailLoading()` to finish loading.
	virtual void assetStartLoading() = 0;

	// Release any data owned by the asset
	virtual void assetUnload() = 0;

	void assetFinishLoading();
	void assetFailLoading();

	// -- Static API

	static void reloadAssetsByName(sf::Slice<const sf::Symbol> names);
	static uint32_t getReloadCount();

	// Find asset by name/props but don't create one if it doesn't exist
	template <typename T>
	static T *find(const sf::Symbol &name) {
		return (T*)impFind(&T::SelfType, name, typename T::PropType{});
	}
	template <typename T>
	static T *find(const sf::Symbol &name, const typename T::PropType &props) {
		sf_assert(sizeof(props) == T::SelfType.propertySize);
		return (T*)impFind(&T::SelfType, name, props);
	}

	// Load an asset by name/props, prefer using Ref<> constructor
	template <typename T>
	static T *load(const sf::Symbol &name) {
		return (T*)impCreate(&T::SelfType, name, typename T::PropType{});
	}
	template <typename T>
	static T *load(const sf::Symbol &name, const typename T::PropType &props) {
		sf_assert(sizeof(props) == T::SelfType.propertySize);
		return (T*)impCreate(&T::SelfType, name, props);
	}

	static uint32_t getNumAssetsLoading();

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
	static void globalUpdate();

	// -- Implementation
	uint32_t impState;     // < Atomic
	uint32_t impFlags;     // < Protected by AssetLibrary::mutex
	uint32_t impFreeFrame; // < Protected by AssetLibrary::mutex

	static Asset *impFind(AssetType *type, const sf::Symbol &name, const AssetProps &props);
	static Asset *impCreate(AssetType *type, const sf::Symbol &name, const AssetProps &props);
};

// Automatic asset reference counting
template <typename T>
struct Ref
{
	T *ptr;
	Ref() : ptr(nullptr) { }
	Ref(const Ref &r) : ptr(r.ptr) { if (r.ptr) r.ptr->retain(); }
	Ref(Ref &&r) : ptr(r.ptr) { r.ptr = nullptr; }
	~Ref() { if (ptr) ptr->release(); }

	// Load asset by name/props
	explicit Ref(const sf::Symbol &name) { ptr = Asset::load<T>(name); }
	explicit Ref(const char *name) { ptr = Asset::load<T>(sf::Symbol(name)); }
	Ref(const sf::Symbol &name, const typename T::PropType &prop) { ptr = Asset::load<T>(name, prop); }

	// Constructor from raw asset pointer
	explicit Ref(T *t) : ptr(t) { }

	Ref &operator=(const Ref &r) {
		if (&r == this) return *this;
		if (ptr) ptr->release();
		if (r.ptr) r.ptr->retain();
		ptr = r.ptr;
		return *this;
	}

	Ref &operator=(Ref &&r) {
		if (&r == this) return *this;
		if (ptr) ptr->release();
		ptr = r.ptr;
		r.ptr = nullptr;
		return *this;
	}

	T *operator->() const { return ptr; }
	explicit operator bool() const { return ptr != nullptr; }
	bool operator!() const { return ptr == nullptr; }
	operator T*() const { return ptr; }

	void reset() {
		if (ptr) {
			ptr->release();
			ptr = nullptr;
		}
	}

	void reset(T *newPtr) {
		if (newPtr) newPtr->retain();
		if (ptr) ptr->release();
		ptr = newPtr;
	}

	void load(const sf::Symbol &name) {
		if (ptr) ptr->release();
		ptr = Asset::load<T>(name);
	}

	void load(const sf::Symbol &name, const typename T::PropType &prop) {
		if (ptr) ptr->release();
		ptr = Asset::load<T>(name, prop);
	}

	void load(const char *name) {
		load(sf::Symbol(name));
	}

	bool isLoading() const { return ptr && !ptr->isLoaded() && !ptr->isFailed(); }
	bool isLoaded() const { return ptr && ptr->isLoaded(); }
};

}

