#include "Asset.h"

#include "sf/ext/mx/mx_platform.h"
#include "sf/HashMap.h"
#include "sf/Array.h"
#include "sf/Mutex.h"
#include "sf/HashSet.h"

namespace sp {

// How many frames to wait before actually deleting assets
const uint32_t FreeQueueFrames = 16;

// #define sp_asset_log(...) sf::debugPrintLine(__VA_ARGS__)
#define sp_asset_log(...) (void)0

enum class LoadState
{
	Unloaded,
	Loading,
	Loaded,
	Failed,
};

enum AssetFlags
{
	// The asset has been freed
	Flag_Freed = 1,
};

struct AssetKey
{
	sf::Symbol name;
	const AssetProps *props;

	bool operator==(const AssetKey &rhs) const
	{
		if (name != rhs.name) return false;
		return props->equal(*rhs.props);
	}
};

uint32_t hash(const AssetKey &key) {
	uint32_t h = sf::hash(key.name);
	h = sf::hashCombine(h, key.props->hash());
	return h;
}

// NOTE: This is zero-initialized!
struct AssetTypeImp
{
	bool inList;

	// Mapping from name+props to assets
	sf::HashMap<AssetKey, Asset*> assetMap;
};

static_assert(sizeof(AssetType::impData) >= sizeof(AssetTypeImp), "impData too small");
static_assert(sizeof(AssetType::impData) <= sizeof(AssetTypeImp) * 4, "impData too large");

struct AssetContext
{
	sf::Mutex mutex;
	sf::Array<Asset*> assetsToFree;
	sf::Array<AssetTypeImp*> types;
	uint32_t frameIndex = 0;
	uint32_t reloadCount = 0;

	// Number of assets loading at the moment
	uint32_t numAssetsLoading = 0;

	void reloadByName(const sf::HashSet<sf::Symbol> &names)
	{
		sf::MutexGuard mg(mutex);
		for (AssetTypeImp *type : types) {
			for (auto &pair : type->assetMap) {
				if (names.find(pair.val->name)) {
					pair.val->startReloading();
				}
			}
		}
		mxa_inc32_rel(&reloadCount);
	}

	Asset *findAsset(AssetType *type, const sf::Symbol &name, const AssetProps &props)
	{
		sf::MutexGuard mg(mutex);
		AssetTypeImp *typeImp = (AssetTypeImp*)type->impData;

		// Look up the asset in the type's asset map
		AssetKey key = { name, &props };
		auto pair = typeImp->assetMap.find(key);

		// If found increase refcount and return the asset
		if (pair) {
			Asset *asset = pair->val;
			asset->retain();
			return asset;
		} else {
			return nullptr;
		}
	}

	Asset *createAsset(AssetType *type, const sf::Symbol &name, const AssetProps &props)
	{
		sf::MutexGuard mg(mutex);
		AssetTypeImp *typeImp = (AssetTypeImp*)type->impData;

		if (!typeImp->inList) {
			typeImp->inList = true;
			types.push(typeImp);
		}

		// Insert the asset to the type's asset map
		// NOTE: The inserted `AssetKey` will refer to local data,
		// but it will be reassigned before leaving the mutex!
		AssetKey key = { name, &props };
		auto result = typeImp->assetMap.insert(key);

		// Early return if found: Bump refcount and return existing asset
		if (!result.inserted) {
			Asset *asset = result.entry.val;
			asset->retain();
			return asset;
		}

		// Allocate the asset and name/props
		size_t size = type->instanceSize + type->propertySize;
		Asset *asset = (Asset*)sf::memAlloc(size);
		AssetProps *propCopy = (AssetProps*)((char*)asset + type->instanceSize);

		// Copy name/props
		props.copyTo(propCopy);

		// Construct the asset
		type->ctorFn(asset);
		asset->type = type;
		asset->name = name;
		asset->props = propCopy;

		// Patch the key and value, propSize should be fine from the insert
		result.entry.key.name = asset->name;
		result.entry.key.props = propCopy;
		result.entry.val = asset;

		sp_asset_log("Create: %s %s", type->name, asset->name.data);

		return asset;
	}

	void addPendingFree(Asset *asset)
	{
		sf::MutexGuard mg(mutex);
		asset->impFreeFrame = frameIndex;
		if (asset->impFlags & Flag_Freed) return;
		asset->impFlags |= Flag_Freed;

		sp_asset_log("Queue free: %s %s", asset->type->name, asset->name.data);

		assetsToFree.push(asset);
	}

	void update()
	{
		sf::MutexGuard mg(mutex);
		frameIndex++;

		for (uint32_t index = 0; index < assetsToFree.size; index++) {
			Asset *asset = assetsToFree[index];

			// Still waiting in the free queue
			if (frameIndex - asset->impFreeFrame < FreeQueueFrames) {
				continue;
			}

			// Remove the asset from the free list if it has been revived
			if (mxa_load32_nf(&asset->refcount) > 0) {
				asset->impFlags &= ~Flag_Freed;
				assetsToFree.removeSwap(index--);
				continue;
			}

			// Wait for the asset to finish loading
			LoadState state = (LoadState)mxa_load32_nf(&asset->impState);
			if (state == LoadState::Loading) {
				continue;
			}

			// Unload the asset if necessary
			if (state == LoadState::Loaded) {
				if (mxa_cas32_nf(&asset->impState, (uint32_t)LoadState::Loaded, (uint32_t)LoadState::Unloaded)) {
					sp_asset_log("Unload: %s %s", asset->type->name, asset->name.data);
					mutex.unlock();
					asset->assetUnload();
					mutex.lock();
				} else {
					continue;
				}
			}

			sp_asset_log("Delete: %s %s", asset->type->name, asset->name.data);

			// Remove the asset from the map
			AssetTypeImp *typeImp = (AssetTypeImp*)asset->type->impData;
			AssetKey key = { asset->name, asset->props };
			typeImp->assetMap.remove(key);

			// Call the virtual destructor and free memory
			asset->props->~AssetProps();
			asset->~Asset();
			sf::memFree(asset);

			// Remove from the free list
			assetsToFree.removeSwap(index--);
		}
	}
};

AssetContext g_assetContext;

AssetProps::~AssetProps()
{
}

uint32_t NoAssetProps::hash() const
{
	return 0;
}

bool NoAssetProps::equal(const AssetProps &rhs) const
{
	return true;
}

void NoAssetProps::copyTo(AssetProps *uninitDst) const
{
	new (uninitDst) NoAssetProps();
}

Asset::Asset()
	: type(nullptr)
	, props(nullptr)
	, refcount(1)
	, impState((uint32_t)LoadState::Unloaded)
	, impFlags(0)
{
}

Asset::~Asset()
{
}

void Asset::retain()
{
	mxa_inc32_nf(&refcount);
}

void Asset::release()
{
	uint32_t count = mxa_dec32_nf(&refcount) - 1;
	if (count == 0) {
		g_assetContext.addPendingFree(this);
	}
}

bool Asset::isLoaded() const
{
	sf_assert(refcount > 0);

	LoadState s = (LoadState)mxa_load32_acq(&impState);
	return s == LoadState::Loaded;
}

bool Asset::isFailed() const
{
	sf_assert(refcount > 0);

	LoadState s = (LoadState)mxa_load32_acq(&impState);
	return s == LoadState::Failed;
}

bool Asset::shouldBeLoaded()
{
	sf_assert(refcount > 0);

	LoadState s = (LoadState)mxa_load32_acq(&impState);
	if (s == LoadState::Loaded) return true;
	if (s == LoadState::Unloaded) {
		sf::debugPrintLine("Asset not loaded before use: %s %s", type->name, name.data);
		startLoading();
	}
	return false;
}

void Asset::startLoading()
{
	sf_assert(refcount > 0);

	// Transition from Unloaded -> Loading
	if (!mxa_cas32_acq(&impState, (uint32_t)LoadState::Unloaded, (uint32_t)LoadState::Loading)) return;
	sp_asset_log("Start load: %s %s", type->name, name.data);

	// Increment the number of assets loading
	mxa_inc32_rel(&g_assetContext.numAssetsLoading);

	assetStartLoading();
}

static void resetAssetImp(Asset *asset)
{
	alignas(Asset) char copyData[sizeof(Asset)];
	memcpy(copyData, (void*)asset, sizeof(Asset));
	Asset *copy = (Asset*)copyData;

	asset->~Asset();
	copy->type->ctorFn(asset);
	asset->type = copy->type;
	asset->name = copy->name;
	asset->props = copy->props;
	asset->refcount = copy->refcount;
	asset->impState = copy->impState;
	asset->impFlags = copy->impFlags;
}

void Asset::startReloading()
{
	// Transition from Loaded -> Loading
	if (mxa_cas32_acq(&impState, (uint32_t)LoadState::Loaded, (uint32_t)LoadState::Loading)) {
		assetUnload();
		resetAssetImp(this);
		assetStartLoading();
		return;
	}

	// Transition from Failed -> Loading
	if (mxa_cas32_acq(&impState, (uint32_t)LoadState::Failed, (uint32_t)LoadState::Loading)) {
		resetAssetImp(this);
		assetStartLoading();
		return;
	}
}

void Asset::assetFinishLoading()
{
	// Transition from Loading -> Loaded (must succeed)
	bool res = mxa_cas32_rel(&impState, (uint32_t)LoadState::Loading, (uint32_t)LoadState::Loaded);
	sf_assertf(res, "Invalid state transition");
	mxa_dec32_rel(&g_assetContext.numAssetsLoading);

	sp_asset_log("Finish load: %s %s", type->name, name.data);
}

void Asset::assetFailLoading()
{
	// Transition from Loading -> Failed (must succeed)
	bool res = mxa_cas32_rel(&impState, (uint32_t)LoadState::Loading, (uint32_t)LoadState::Failed);
	sf_assertf(res, "Invalid state transition");
	mxa_dec32_rel(&g_assetContext.numAssetsLoading);

	sp_asset_log("Fail load: %s %s", type->name, name.data);
}

uint32_t Asset::getNumAssetsLoading()
{
	return mxa_load32_acq(&g_assetContext.numAssetsLoading);
}

void Asset::globalInit()
{
}

void Asset::globalCleanup()
{
}

void Asset::globalUpdate()
{
	g_assetContext.update();
}

void Asset::reloadAssetsByName(sf::Slice<const sf::Symbol> names)
{
	sf::HashSet<sf::Symbol> set;
	for (const sf::Symbol &name : names) set.insert(name);
	g_assetContext.reloadByName(set);
}

uint32_t Asset::getReloadCount()
{
	return mxa_load32_acq(&g_assetContext.reloadCount);
}

Asset *Asset::impFind(AssetType *type, const sf::Symbol &name, const AssetProps &props)
{
	return g_assetContext.findAsset(type, name, props);
}

Asset *Asset::impCreate(AssetType *type, const sf::Symbol &name, const AssetProps &props)
{
	Asset *asset = g_assetContext.createAsset(type, name, props);

	// TODO: Manual loading
	asset->startLoading();

	return asset;

}

}
