#include "Asset.h"

#include "sf/ext/mx/mx_platform.h"
#include "sf/HashMap.h"
#include "sf/Array.h"

const uint32_t FreeQueueFrames = 60;

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

struct AssetToFree
{
	Asset *asset;
	uint32_t frameIndex;
};

struct AssetLibrary
{
	sf::Mutex mutex;
	sf::HashMap<sf::String, Asset*> assetsByName;
	sf::Array<AssetToFree> assetsToFree;
	uint32_t frameIndex;

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

		// Allocate the asset
		Asset *asset = (Asset*)sf::memAlloc(type->size);

		// Call the asset type constructor, patch the name,
		// and finally patch `result.entry` to point to
		// the asset and its name.
		type->init(asset);
		asset->type = type;
		asset->name = name;
		result.entry.key = asset->name;
		result.entry.val = asset;
		return asset;
	}

	void addPendingFree(Asset *asset)
	{
		sf::MutexGuard mg(mutex);
		if (asset->flags & Flag_Freed) return;
		asset->flags |= Flag_Freed;

		AssetToFree &pendingFree = assetsToFree.push();
		pendingFree.asset = asset;
		pendingFree.frameIndex = frameIndex;
	}

	void update()
	{
		sf::MutexGuard mg(mutex);
		frameIndex++;

		for (size_t index = 0; index < assetsToFree.size; index++) {
			AssetToFree &pendingFree = assetsToFree[index];
			Asset *asset = pendingFree.asset;

			if (frameIndex - pendingFree.frameIndex < FreeQueueFrames) {
				// Asset is still waiting to be freed
				continue;
			}

			if (mxa_load32_nf(&asset->refcount) > 0) {
				// Asset has been revived
				assetsToFree.removeSwap(index);
				index--;
				continue;
			}

			// TODO: Unload

			// Remove the asset from the map
			sf::String name = asset->name;
			assetsByName.remove(name);
			asset->~Asset();
			sf::memFree(asset);
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

bool Asset::shouldBeLoaded()
{
	sf_assert(refcount > 0);
	LoadState s = (LoadState)mxa_load32_acq(&state);
	if (s == LoadState::Loaded) return true;
	if (s == LoadState::Unloaded) {
		sf::debugPrintLine("Asset not loaded before use: %s %s", type->name, name.data);
		startLoading();
	}
}

void Asset::startLoading()
{
	if (!mxa_cas32_nf(&state, (uint32_t)LoadState::Unloaded, (uint32_t)LoadState::Loading)) return;
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
}

void Asset::freeImp()
{
}

}
