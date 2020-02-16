#include "Asset.h"

#include "sf/Mutex.h"
#include "sf/Array.h"
#include "sf/HashMap.h"

#include "ext/sokol/sokol_fetch.h"

static const uint32_t LoadBufferSize = 4 * 1024 * 1024;
static const uint32_t NumLoadBuffers = 4;

struct AssetInfo
{
	sf::StringBuf name;
	uint32_t refcount = 0;

	sfetch_handle_t fetchHandle;
};

struct TypeInfo
{
	AssetType *type = nullptr;

	sf::HashMap<sf::String, uint32_t> assetMap;
	sf::Array<AssetInfo> assets;
	sf::Array<uint32_t> freeList;
};

struct AssetData
{
	sf::Mutex mutex;
	sf::Array<TypeInfo> types;

	sf::Array<Asset> assetsToLoad;
	sf::Array<char> loadBuffers[NumLoadBuffers];
};
static AssetData g_data;

static void initAssetType(AssetData &data, AssetType *type)
{
	// Null asset type at index 0
	if (data.types.size == 0) {
		data.types.push();
	}

	uint32_t index = data.types.size;

	TypeInfo &info = data.types.push();
	info.type = type;
	type->typeIndex = index;
}

Asset loadAssetImp(sf::String name, AssetType *type)
{
	AssetData &data = g_data;
	const sf::MutexGuard mg(data.mutex);

	if (type->typeIndex == 0) {
		initAssetType(data, type);
	}

	TypeInfo &typeInfo = data.types[type->typeIndex];
	auto result = typeInfo.assetMap.insert(name, ~0u);
	sf::KeyVal<sf::String, uint32_t> &entry = result.entry;
	uint32_t assetIndex;
	bool needLoading = false;

	if (result.inserted) {
		if (typeInfo.freeList.size > 0) {
			assetIndex = typeInfo.freeList.popValue();
		} else {
			assetIndex = typeInfo.assets.size;
			typeInfo.assets.push();
		}
		AssetInfo &assetInfo = typeInfo.assets[assetIndex];

		assetInfo.name = name;
		assetInfo.refcount = 1;

		// HACK: Patch `entry` key to point to heap allocated string and asset index
		entry.key = assetInfo.name;
		entry.val = assetIndex;

		needLoading = true;
	} else {
		assetIndex = entry.val;
		typeInfo.assets[assetIndex].refcount++;
	}

	Asset asset = Asset(type->typeIndex, assetIndex);

	if (needLoading) {
		data.assetsToLoad.push(asset);
	}

	return asset;
}

static void fetchCallback(const sfetch_response_t *response)
{
	AssetData &data = g_data;

	// Should be called only from `updateAssets()`
	sf_assert(data.mutex.isLocked());

	sfetch_handle_t handle = response->handle;

	Asset asset = *(const Asset*)response->user_data;
	TypeInfo &typeInfo = data.types[asset.typeIndex()];
	AssetInfo &assetInfo = typeInfo.assets[asset.assetIndex()];
	sf_assert(assetInfo.fetchHandle.id == handle.id);

	if (response->dispatched) {
		void *ptr = data.loadBuffers[response->lane].data;
		sfetch_bind_buffer(handle, ptr, LoadBufferSize);
	}
	if (response->fetched) {
		AssetLoadInfo loadInfo;
		loadInfo.name = assetInfo.name;
		loadInfo.data = response->buffer_ptr;
		loadInfo.size = response->fetched_size;
		typeInfo.type->load(asset, loadInfo);
	}
}

static bool startLoadingAsset(AssetData &data, Asset asset)
{
	TypeInfo &typeInfo = data.types[asset.typeIndex()];
	AssetInfo &assetInfo = typeInfo.assets[asset.assetIndex()];

	sfetch_request_t req = { };
	req.path = assetInfo.name.data;
	req.callback = &fetchCallback;
	req.user_data_ptr = &asset;
	req.user_data_size = sizeof(Asset);

	sfetch_handle_t handle = sfetch_send(&req);
	if (!sfetch_handle_valid(handle)) return false;

	assetInfo.fetchHandle = handle;
	return true;
}

void setupAssets()
{
	AssetData &data = g_data;
	const sf::MutexGuard mg(data.mutex);

	sfetch_desc_t desc = { };
	desc.num_channels = 1;
	desc.num_lanes = NumLoadBuffers;
	sfetch_setup(&desc);

	for (auto &buf : data.loadBuffers) {
		buf.resizeUninit(LoadBufferSize);
	}
}

void updateAssets()
{
	AssetData &data = g_data;
	const sf::MutexGuard mg(data.mutex);

	// Start new loads if possible
	while (data.assetsToLoad.size > 0) {
		if (!startLoadingAsset(data, data.assetsToLoad.back())) {
			break;
		}
		data.assetsToLoad.pop();
	}

	// Invoke callbacks
	sfetch_dowork();
}
