#include "ContentFile.h"

#include "sf/Mutex.h"
#include "sf/Array.h"
#include "sf/HashMap.h"

#include "ext/sokol/sokol_fetch.h"

namespace sp {

static constexpr uint32_t NumBuffers = 4;
static constexpr uint32_t BufferSize = 4 * 1024 * 1024;

// From EmbeddedFiles.cpp
ContentPackage *getEmbeddedContentPackage();

struct PendingFile
{
	PendingFile() = default;
	PendingFile(PendingFile&&) = default;
	PendingFile(const PendingFile&) = delete;

	ContentLoadHandle handle;
	sf::StringBuf name;
	ContentFile::Callback callback = nullptr;
	void *user = nullptr;
	bool cancelled = false;

	uint32_t stage = 0;
	ContentPackage *currentPackage = nullptr;
	ContentFile file;
};

struct FetchFilePackage;

struct ContentFileContext
{
	sf::Mutex mutex;
	sf::HashMap<uint32_t, PendingFile> files;
	uint32_t nextId = 0;

	sf::Array<FetchFilePackage*> fetchPackages;
	sf::Array<ContentPackage*> packages;

	sf::Array<char> fetchBuffers[NumBuffers];
};

ContentFileContext g_contentFileContext;

static void fetchCallback(const sfetch_response_t *response)
{
	ContentLoadHandle handle = *(ContentLoadHandle*)response->user_data;

	if (response->failed) {
		// Failed: Return with empty file
		ContentFile::packageFileFailed(handle);
	} else if (response->finished) {
		// Finished: Call callback with actual data
		ContentFile::packageFileLoaded(handle, response->buffer_ptr, response->fetched_size);
	} else if (response->dispatched) {
		// Dispatched: Allocate a buffer, no need for mutex
		// since all the callbacks are from the same thread
		ContentFileContext &ctx = g_contentFileContext;
		void *ptr = ctx.fetchBuffers[response->lane].data;
		sfetch_bind_buffer(response->handle, ptr, BufferSize);
	}
}

ContentPackage::~ContentPackage()
{
}

struct FetchFilePackage : ContentPackage
{
	sf::StringBuf root;

	FetchFilePackage(const sf::String &root)
		: root(root)
	{
	}

	virtual bool shouldTryToLoad(const sf::String &name) final
	{
		// TODO: Separate absolute and relative files?
		return true;
	}

	virtual bool startLoadingFile(ContentLoadHandle handle, const sf::String &name) final
	{
		sf::SmallStringBuf<256> path;
		path.append(root, name);

		sfetch_request_t req = { };
		req.callback = &fetchCallback;
		req.path = path.data;
		req.user_data_ptr = &handle;
		req.user_data_size = sizeof(ContentLoadHandle);
		sfetch_handle_t fetchHandle = sfetch_send(&req);
		return sfetch_handle_valid(fetchHandle);
	}
};

void ContentFile::addRelativeFileRoot(const sf::String &root)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	FetchFilePackage *package = new FetchFilePackage(root);
	ctx.packages.push(package);
	ctx.fetchPackages.push(package);
}

ContentLoadHandle ContentFile::load(const sf::String &name, Callback callback, void *user)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	uint32_t id = ++ctx.nextId;
	if (id >= 0x0fffffff) {
		// Wrap around before 0x10000000
		ctx.nextId = 0;
	}

	PendingFile &file = ctx.files[id];

	ContentLoadHandle handle;
	handle.id = id;

	file.handle = handle;
	file.name = name;
	file.callback = callback;
	file.user = user;

	return handle;
}

void ContentFile::cancel(ContentLoadHandle handle)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	auto pair = ctx.files.find(handle.id);
	if (pair == nullptr) return;
	PendingFile &file = pair->val;	

	file.cancelled = true;
}

void ContentFile::packageFileLoaded(ContentLoadHandle handle, const void *data, size_t size, bool stableData)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	auto pair = ctx.files.find(handle.id);
	sf_assert(pair != nullptr);
	PendingFile &file = pair->val;	

	file.file.package = file.currentPackage;
	file.file.stableData = stableData;
	file.file.size = size;
	if (stableData) {
		file.file.data = data;
	} else {
		file.file.data = sf::memAlloc(size);
		memcpy((void*)file.file.data, data, size);
	}

	file.currentPackage = nullptr;
}

void ContentFile::packageFileFailed(ContentLoadHandle handle)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	auto pair = ctx.files.find(handle.id);
	sf_assert(pair != nullptr);
	PendingFile &file = pair->val;	

	file.currentPackage = nullptr;
}

void ContentFile::globalInit()
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	for (auto &buffer : ctx.fetchBuffers) {
		buffer.resizeUninit(BufferSize);
	}

	// Insert embedded content package as first always
	ctx.packages.push(getEmbeddedContentPackage());

	sfetch_desc_t desc = { };
	desc.num_lanes = NumBuffers;
	sfetch_setup(&desc);
}

void ContentFile::globalCleanup()
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	sfetch_shutdown();

	for (FetchFilePackage *package : ctx.fetchPackages) {
		delete package;
	}
}

struct PendingLoad
{
	sf::String name; // < Name points to PendingFile::name which is stable w.r.t. moving
	ContentLoadHandle handle;
	ContentPackage *package;
};

void ContentFile::globalUpdate()
{
	ContentFileContext &ctx = g_contentFileContext;

	sfetch_dowork();

	sf::SmallArray<PendingFile, 16> doneFiles;
	sf::SmallArray<PendingLoad, 32> loads;

	// Update requests
	{
		sf::MutexGuard mg(ctx.mutex);

		for (auto it = ctx.files.begin(); it != ctx.files.end(); ) {
			PendingFile &file = it->val;

			// Waiting for package callback
			if (file.currentPackage != nullptr) {
				++it;
				continue;
			}

			// Remove from the list if the load has been canelled
			if (file.cancelled) {
				it = ctx.files.removeAt(it);
				continue;
			}

			// Report success if the file was loaded
			if (file.file.isValid()) {
				doneFiles.push(std::move(file));
				it = ctx.files.removeAt(it);
				continue;
			}

			// Load from the next source
			while (file.stage < ctx.packages.size) {
				ContentPackage *package = ctx.packages[file.stage++];
				if (!package->shouldTryToLoad(file.name)) {
					// Don't even try to load from this package
					continue;
				}

				// Queue the load
				loads.push({ file.name, file.handle, package });
				file.currentPackage = package;
				break;
			}

			if (file.stage >= ctx.packages.size && file.currentPackage == nullptr) {
				// Tried to load from all sources, remove from the list
				// and report failure later outside of the mutex
				doneFiles.push(std::move(file));
				it = ctx.files.removeAt(it);
				continue;
			}

			// Advance to next file
			++it;
		}
	}

	// Call callbacks outside the mutex
	for (PendingFile &file : doneFiles) {
		file.callback(file.user, file.file);

		if (!file.file.stableData) {
			sf::memFree((void*)file.file.data);
		}
	}
	for (PendingLoad &load : loads) {
		if (!load.package->startLoadingFile(load.handle, load.name)) {
			// Failed loading, roll back currentPackage and stage
			sf::MutexGuard mg(ctx.mutex);

			auto pair = ctx.files.find(load.handle.id);
			sf_assert(pair != nullptr);
			PendingFile &file = pair->val;	
			file.currentPackage = nullptr;
			file.stage--;

		}
	}
}

// Fetch


#if 0


void ContentFile::addFileRoot(const sf::String &root)
{
}

ContentFile::LoadHandle ContentFile::load(const sf::String &name, Callback callback, void *user)
{
	ContentFileContext &ctx = g_contentFileContext;

	ContentFileInfo info;
	info.callback = callback;
	info.user = user;

	sf::SmallStringBuf<256> path;
	path.append(ctx.root, name);

	sfetch_request_t req = { };
	req.callback = &fetchCallback;
	req.path = path.data;
	req.user_data_ptr = &info;
	req.user_data_size = sizeof(ContentFileInfo);
	sfetch_handle_t handle = sfetch_send(&req);
	sf_assert(sfetch_handle_valid(handle));

	LoadHandle lh;
	lh.id = handle.id;
	return lh;
}

void ContentFile::cancel(ContentFile::LoadHandle handle)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	sfetch_handle_t h = { handle.id };
	if (!sfetch_handle_valid(h)) return;

	ctx.cancelledHandles.push(h);
	sfetch_cancel(h);
}
#endif

}

