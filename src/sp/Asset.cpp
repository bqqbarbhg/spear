#include "Asset.h"

#include "sf/ext/mx/mx_platform.h"
#include "sf/HashMap.h"
#include "sf/Array.h"

const uint32_t UnloadQueueFrames = 4;
const uint32_t FreeQueueFrames = 16;

#define sp_asset_log(...) sf::debugPrintLine(__VA_ARGS__)

enum class LoadState
{
	Unloaded,
	Loading,
	Loaded,
	Failed,
};

enum AssetFlags
{
	Flag_Freed = 1,
};

namespace sp {

#define sp_assert_type(type, expected) \
	sf_assertf((type) == (expected), \
		"Asset type mismatch: Expected %u, but asset is %u", \
		(expected)->name, (type)->name)

struct PendingAsset
{
	Asset *asset;
	uint32_t frameIndex;
};

struct AssetLibrary
{
	sf::Mutex mutex;
	sf::HashMap<sf::String, Asset*> assetsByName;
	sf::Array<PendingAsset> assetsToFree;
	uint32_t frameIndex = 0;
	uint32_t numAssetsLoading = 0;

	Asset *findAsset(const sf::String &name, const AssetType *type)
	{
		sf::MutexGuard mg(mutex);
		auto pair = assetsByName.find(name);
		if (pair) {
			Asset *asset = pair->val;
			sp_assert_type(asset->type, type);
			asset->retain();
			return asset;
		} else {
			return nullptr;
		}
	}

	Asset *createAsset(const sf::String &name, const AssetType *type)
	{
		sf::MutexGuard mg(mutex);

		// Insert the assert with key referencing local `name`
		// and no asset pointer (!).
		auto result = assetsByName.insert(name);

		// Early return if it already exists
		if (!result.inserted) {
			Asset *asset = result.entry.val;
			sp_assert_type(asset->type, type);
			asset->retain();
			return asset;
		}

		// Allocate the asset and space for the name
		Asset *asset = (Asset*)sf::memAlloc(type->size + name.size + 1);
		char *nameCopy = (char*)asset + type->size;
		memcpy(nameCopy, name.data, name.size);
		nameCopy[name.size] = '\0';

		// Call the asset type constructor, patch the name,
		// and finally patch `result.entry` to point to
		// the asset and its name.
		type->init(asset);
		asset->type = type;
		asset->name = sf::CString(nameCopy, name.size);
		result.entry.key = asset->name;
		result.entry.val = asset;

		sp_asset_log("Create: %s %s", type->name, asset->name.data);

		// TODO: Manual loading
		asset->startLoading();

		return asset;
	}

	void addPendingFree(Asset *asset)
	{
		sf::MutexGuard mg(mutex);
		if (asset->flags & Flag_Freed) return;
		asset->flags |= Flag_Freed;

		sp_asset_log("Queue free: %s %s", asset->type->name, asset->name.data);

		PendingAsset &pendingFree = assetsToFree.push();
		pendingFree.asset = asset;
		pendingFree.frameIndex = frameIndex;
	}

	void update()
	{
		sf::MutexGuard mg(mutex);
		frameIndex++;

		for (uint32_t index = 0; index < assetsToFree.size; index++) {
			PendingAsset &pendingFree = assetsToFree[index];
			Asset *asset = pendingFree.asset;

			if (frameIndex - pendingFree.frameIndex < FreeQueueFrames) {
				// Asset is still waiting to be freed
				continue;
			}

			if (mxa_load32_nf(&asset->refcount) > 0) {
				// Asset has been revived
				asset->flags &= ~Flag_Freed;
				assetsToFree.removeSwap(index--);
				continue;
			}

			LoadState state = (LoadState)mxa_load32_nf(&asset->state);
			if (state == LoadState::Loading) {
				// Wait for the asset to finish loading
				continue;
			}

			// Unload the asset if necessary
			if (state == LoadState::Loaded) {
				if (mxa_cas32_nf(&asset->state, (uint32_t)LoadState::Loaded, (uint32_t)LoadState::Unloaded)) {
					sp_asset_log("Unload: %s %s", asset->type->name, asset->name.data);
					asset->unloadImp();
				} else {
					continue;
				}
			}

			sp_asset_log("Delete: %s %s", asset->type->name, asset->name.data);

			// Remove the asset from the map
			sf::String name = asset->name;
			assetsByName.remove(name);
			asset->~Asset();
			sf::memFree(asset);

			assetsToFree.removeSwap(index--);
		}
	}
};

AssetLibrary g_library;

Asset::Asset()
	: type(nullptr)
	, state((uint32_t)LoadState::Unloaded)
	, refcount(1)
	, flags(0)
{
}

Asset::~Asset()
{
}

bool Asset::isLoaded() const
{
	sf_assert(refcount > 0);
	LoadState s = (LoadState)mxa_load32_acq(&state);
	return s == LoadState::Loaded;
}

bool Asset::isFailed() const
{
	sf_assert(refcount > 0);
	LoadState s = (LoadState)mxa_load32_acq(&state);
	return s == LoadState::Failed;
}

bool Asset::shouldBeLoaded()
{
	sf_assert(refcount > 0);
	LoadState s = (LoadState)mxa_load32_acq(&state);
	if (s == LoadState::Loaded) return true;
	if (s == LoadState::Unloaded) {
		sf::debugPrintLine("Asset not loaded before use: %s %s", type->name, name.data);
		startLoading();
	}
	return false;
}

void Asset::startLoading()
{
	if (!mxa_cas32_acq(&state, (uint32_t)LoadState::Unloaded, (uint32_t)LoadState::Loading)) return;
	mxa_inc32_rel(&g_library.numAssetsLoading);
	sp_asset_log("Start load: %s %s", type->name, name.data);
	startLoadingImp();
}

void Asset::retain()
{
	mxa_inc32_nf(&refcount);
}

void Asset::release()
{
	uint32_t count = mxa_dec32_nf(&refcount) - 1;
	if (count == 0) {
		g_library.addPendingFree(this);
	}
}

Asset *Asset::findImp(const sf::String &name, const AssetType *type)
{
	sf_assert(type != nullptr);
	return g_library.findAsset(name, type);
}

Asset *Asset::createImp(const sf::String &name, const AssetType *type)
{
	sf_assert(type != nullptr);
	return g_library.createAsset(name, type);
}

void Asset::finishLoadingImp()
{
	bool res = mxa_cas32_rel(&state, (uint32_t)LoadState::Loading, (uint32_t)LoadState::Loaded);
	sf_assertf(res, "Invalid state transition");
	mxa_dec32_rel(&g_library.numAssetsLoading);
	sp_asset_log("Finish load: %s %s", type->name, name.data);
}

void Asset::failLoadingImp()
{
	// sf::debugPrintLine("Failed to load: %s %s", type->name, name.data);
	bool res = mxa_cas32_rel(&state, (uint32_t)LoadState::Loading, (uint32_t)LoadState::Failed);
	sf_assertf(res, "Invalid state transition");
	mxa_dec32_rel(&g_library.numAssetsLoading);
	sp_asset_log("Fail load: %s %s", type->name, name.data);
}

void Asset::init()
{
}

void Asset::update()
{
	g_library.update();
}

uint32_t Asset::getNumAssetsLoading()
{
	return mxa_load32_acq(&g_library.numAssetsLoading);
}

}
