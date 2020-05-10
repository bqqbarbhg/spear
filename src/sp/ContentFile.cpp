#include "ContentFile.h"

#include "sf/Mutex.h"
#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sf/Thread.h"
#include "sf/Semaphore.h"
#include "sf/File.h"

#include "ext/sokol/sokol_fetch.h"

#if SF_OS_EMSCRIPTEN
	#include <emscripten/emscripten.h>
#endif

namespace sp {

// #define sp_file_log(...) sf::debugPrintLine(__VA_ARGS__)
#define sp_file_log(...) (void)0

static constexpr uint32_t NumLanes = 4;
static constexpr uint32_t BufferSize = 16 * 1024 * 1024;

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
	bool mainThread = false;

	uint32_t stage = 0;
	ContentPackage *currentPackage = nullptr;
	ContentFile file;
};

struct ContentFileContext
{
	sf::Mutex mutex;
	sf::HashMap<uint32_t, PendingFile> files;
	uint32_t nextId = 0;

	sf::Array<ContentPackage*> packagesToDelete;
	sf::Array<ContentPackage*> packages;

	sf::Array<char> fetchBuffers[NumLanes];
	sf::Array<PendingFile> mainThreadDoneFiles;

	sf::Semaphore workerSemaphore;
	sf::Thread *workerThread = nullptr;
	bool joinThread = false;
};

ContentFileContext g_contentFileContext;

ContentPackage::~ContentPackage()
{
}

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

struct FetchFilePackage : ContentPackage
{
	sf::StringBuf root;

	FetchFilePackage(const sf::String &root_)
		: root(root_)
	{
		if (root.size > 0) {
			char c = root.data[root.size - 1];
			if (c != '/' && c != '\\') {
				root.append('/');
			}
		}

		name.append("Fetch(", root_, ")");
	}

	virtual bool shouldTryToLoad(const sf::CString &name) final
	{
		// TODO: Separate absolute and relative files?
		return true;
	}

	virtual bool startLoadingFile(ContentLoadHandle handle, const sf::CString &name) final
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

struct CacheDownloadPackage;

struct FetchCacheData
{
	char *destinationFile;
	ContentLoadHandle handle;
	CacheDownloadPackage *package;
};

static void fetchCacheCallback(const sfetch_response_t *response);

struct CacheDownloadPackage : ContentPackage
{
	sf::Mutex cacheMutex;
	sf::StringBuf root;
	ContentCacheResolveFunc *resolveFunc;
	void *user;

	CacheDownloadPackage(sf::String name_, ContentCacheResolveFunc *resolveFunc, void *user)
		: resolveFunc(resolveFunc), user(user)
	{
		name = name_;
	}

	virtual bool shouldTryToLoad(const sf::CString &name) final
	{
		sf::SmallStringBuf<256> url, path;
		return resolveFunc(name, url, path, user);
	}

	virtual bool startLoadingFile(ContentLoadHandle handle, const sf::CString &name) final
	{
		sf::SmallStringBuf<256> url, path;
		bool ret = resolveFunc(name, url, path, user);
		sf_assert(ret == true);

		sfetch_request_t req = { };

		bool exists = false;

		// Check if the cache file exists behind a mutex
		#if !SF_OS_WASM
		{
			sf::MutexGuard mg(cacheMutex);
			exists = sf::fileExists(path);
		}
		#endif

		FetchCacheData data;

		if (exists) {
			// Load from local file
			req.callback = &fetchCallback;
			req.path = path.data;
			req.user_data_ptr = &handle;
			req.user_data_size = sizeof(ContentLoadHandle);
		} else {
			data.destinationFile = (char*)sf::memAlloc(path.size + 1);
			memcpy(data.destinationFile, path.data, path.size);
			data.destinationFile[path.size] = '\0';
			data.handle = handle;
			data.package = this;

			req.callback = &fetchCacheCallback;
			req.path = url.data;
			req.user_data_ptr = &data;
			req.user_data_size = sizeof(FetchCacheData);
		}

		sfetch_handle_t fetchHandle = sfetch_send(&req);
		return sfetch_handle_valid(fetchHandle);

	}
};

static void fetchCacheCallback(const sfetch_response_t *response)
{
	FetchCacheData *data = (FetchCacheData *)response->user_data;

	if (response->failed) {
		// Failed: Return with empty file
		ContentFile::packageFileFailed(data->handle);
	} else if (response->finished) {
		// Finished: Copy to cache and call callback with actual data

		{
			sf::MutexGuard mg(data->package->cacheMutex);
			for  (const char *end = data->destinationFile; (end = strpbrk(end, "\\/")) != nullptr; end++) { 
				sf::createDirectory(sf::String(data->destinationFile, end - data->destinationFile));
			}
			sf::writeFile(sf::String(data->destinationFile), response->buffer_ptr, response->fetched_size);
		}

		ContentFile::packageFileLoaded(data->handle, response->buffer_ptr, response->fetched_size);
	} else if (response->dispatched) {
		// Dispatched: Allocate a buffer, no need for mutex
		// since all the callbacks are from the same thread
		ContentFileContext &ctx = g_contentFileContext;
		void *ptr = ctx.fetchBuffers[response->lane].data;
		sfetch_bind_buffer(response->handle, ptr, BufferSize);
	}

	// Free the pointer if this is the last callback
	if (response->finished) {
		sf::memFree(data->destinationFile);
	}
}

void ContentFile::addRelativeFileRoot(const sf::String &root)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	FetchFilePackage *package = new FetchFilePackage(root);
	ctx.packages.push(package);
	ctx.packagesToDelete.push(package);
}

void ContentFile::addCacheDownloadRoot(sf::String name, ContentCacheResolveFunc *resolveFunc, void *user)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	CacheDownloadPackage *package = new CacheDownloadPackage(name, resolveFunc, user);
	ctx.packages.push(package);
	ctx.packagesToDelete.push(package);
}

static ContentLoadHandle loadImp(const sf::String &name, ContentFile::Callback callback, void *user, bool mainThread)
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
	file.mainThread = mainThread;

	return handle;
}

ContentLoadHandle ContentFile::loadAsync(const sf::String &name, Callback callback, void *user)
{
	return loadImp(name, callback, user, false);
}

ContentLoadHandle ContentFile::loadMainThread(const sf::String &name, Callback callback, void *user)
{
	return loadImp(name, callback, user, true);
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

	sp_file_log("%s: Loaded %s", file.currentPackage->name.data, file.name.data);

	file.currentPackage = nullptr;
}

void ContentFile::packageFileFailed(ContentLoadHandle handle)
{
	ContentFileContext &ctx = g_contentFileContext;
	sf::MutexGuard mg(ctx.mutex);

	auto pair = ctx.files.find(handle.id);
	sf_assert(pair != nullptr);
	PendingFile &file = pair->val;	

	sp_file_log("%s: Failed %s", file.currentPackage->name.data, file.name.data);

	file.currentPackage = nullptr;
}

static void contentUpdateImp(ContentFileContext &ctx);

static void setupInThread()
{
	sfetch_desc_t desc = { };
	desc.num_lanes = NumLanes;
	sfetch_setup(&desc);
}

static void cleanupInThread()
{
	sfetch_shutdown();
}

#if SF_OS_EMSCRIPTEN
static bool emscriptenSetup = false;

static void contentFileWorkerUpdate(void *arg)
{
	ContentFileContext &ctx = *(ContentFileContext*)arg;
	if (!emscriptenSetup) {
		emscriptenSetup = true;
		setupInThread();
	}

	if (ctx.joinThread) {
		cleanupInThread();
		emscripten_pause_main_loop();
		return;
	}

	contentUpdateImp(ctx);
}
#endif

static void contentFileWorker(void *arg)
{
	ContentFileContext &ctx = *(ContentFileContext*)arg;

#if SF_OS_EMSCRIPTEN

	emscripten_set_main_loop_arg(&contentFileWorkerUpdate, &ctx, 0, false);

#else

	setupInThread();

	for (;;) {
		ctx.workerSemaphore.wait();
		if (ctx.joinThread) break;

		contentUpdateImp(ctx);
	}

	cleanupInThread();

#endif

}

void ContentFile::globalInit(bool useWorker)
{
	ContentFileContext &ctx = g_contentFileContext;

	for (auto &buffer : ctx.fetchBuffers) {
		buffer.resizeUninit(BufferSize);
	}

	// Insert embedded content package as first always
	ctx.packages.push(getEmbeddedContentPackage());

	if (useWorker) {
		sf::ThreadDesc desc;
		desc.entry = &contentFileWorker;
		desc.user = &ctx;
		desc.name = "Content Thread";
		sf::Thread *thread = sf::Thread::start(desc);
		if (thread) {
			ctx.workerThread = thread;
			return;
		}
	}

	// No worker, set up on this thread
	setupInThread();
}

void ContentFile::globalCleanup()
{
	ContentFileContext &ctx = g_contentFileContext;

	if (ctx.workerThread) {
		ctx.joinThread = true;
		ctx.workerSemaphore.signal();
		sf::Thread::join(ctx.workerThread);
	} else {
		cleanupInThread();
	}

	sf::MutexGuard mg(ctx.mutex);

	for (ContentPackage *package : ctx.packagesToDelete) {
		delete package;
	}
}

struct PendingLoad
{
	sf::CString name; // < Name points to PendingFile::name which is stable w.r.t. moving
	ContentLoadHandle handle;
	ContentPackage *package;
};

static void contentUpdateImp(ContentFileContext &ctx)
{
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
				if (file.mainThread && ctx.workerThread) {
					ctx.mainThreadDoneFiles.push(std::move(file));
				} else {
					doneFiles.push(std::move(file));
				}
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
				if (file.mainThread && ctx.workerThread) {
					ctx.mainThreadDoneFiles.push(std::move(file));
				} else {
					doneFiles.push(std::move(file));
				}
				it = ctx.files.removeAt(it);
				continue;
			}

			// Advance to next file
			++it;
		}
	}

	// Call callbacks outside the mutex
	for (PendingFile &file : doneFiles) {
		sp_file_log("Thread Callback (%s) %s", file.file.isValid() ? "OK" : "FAIL", file.name.data);
		file.callback(file.user, file.file);

		if (!file.file.stableData) {
			sf::memFree((void*)file.file.data);
		}
	}

	for (PendingLoad &load : loads) {
		sp_file_log("%s: Start load %s", load.package->name.data, load.name.data);

		if (!load.package->startLoadingFile(load.handle, load.name)) {
			sp_file_log("%s: Interrupted %s", load.package->name.data, load.name.data);

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

void ContentFile::globalUpdate()
{
	ContentFileContext &ctx = g_contentFileContext;
	if (ctx.workerThread) {
		if (ctx.workerSemaphore.getCount() < 4) {
			ctx.workerSemaphore.signal();
		}

		sf::SmallArray<PendingFile, 16> doneFiles;

		// Get done files to process on the main thread
		{
			sf::MutexGuard mg(ctx.mutex);
			doneFiles.reserve(ctx.mainThreadDoneFiles.size);
			for (PendingFile &file : ctx.mainThreadDoneFiles) {
				doneFiles.push(std::move(file));
			}
			ctx.mainThreadDoneFiles.clear();
		}

		// Call callbacks outside the mutex
		for (PendingFile &file : doneFiles) {
			sp_file_log("Main Callback (%s) %s", file.file.isValid() ? "OK" : "FAIL", file.name.data);
			file.callback(file.user, file.file);

			if (!file.file.stableData) {
				sf::memFree((void*)file.file.data);
			}
		}

	} else {
		// Do the update on the main thread
		contentUpdateImp(ctx);

		// Main thread files should be called directly
		sf_assert(ctx.mainThreadDoneFiles.size == 0);
	}
}

}

