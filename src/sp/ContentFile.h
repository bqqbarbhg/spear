#pragma once

#include "sf/String.h"

namespace sp {

struct ContentLoadHandle {
	uint32_t id = ~0u;
	bool isValid() const { return id != ~0u; }
	bool operator==(ContentLoadHandle rhs) const { return id == rhs.id; }
	bool operator!=(ContentLoadHandle rhs) const { return id != rhs.id; }
};

struct ContentPackage
{
	sf::StringBuf name;

	virtual ~ContentPackage();
	virtual bool shouldTryToLoad(const sf::CString &name) = 0;
	virtual bool startLoadingFile(ContentLoadHandle handle, const sf::CString &name) = 0;
};

typedef bool ContentCacheResolveFunc(const sf::CString &name, sf::StringBuf &url, sf::StringBuf &path, void *user);

struct MainThreadCallback
{
	virtual void run() = 0;
};

template <typename Fn>
struct MainThreadCallbackFn final : MainThreadCallback
{
	Fn fn;

	sf_forceinline MainThreadCallbackFn(Fn fn) : fn(fn) { }
	virtual void run() final override { fn(); }
};

struct ContentFile
{
	typedef void (*Callback)(void *user, const ContentFile &file);

	sf_forceinline bool isValid() const { return data != nullptr; }

	const void *data = nullptr;        // < File data pointer
	size_t size = 0;                   // < File size in bytes
	bool stableData = false;           // < If true `data` will remain valid after the call
	ContentPackage *package = nullptr; // < Package the file was found in

	// -- Static API

	// Add a file path to load from
	static void addRelativeFileRoot(const sf::String &root, const sf::String &prefix=sf::String());
	static void addCacheDownloadRoot(sf::String name, ContentCacheResolveFunc *resolveFunc, void *user);

	// Add a custom package type
	static void addContentPackage(ContentPackage *package);

	// Package functions
	static void packageFileLoaded(ContentLoadHandle handle, const void *data, size_t size, bool stableData=false);
	static void packageFileFailed(ContentLoadHandle handle);

	// Load a resource from `name`.
	// Returns a handle that can be passed to `cancel()`.
	static ContentLoadHandle loadAsync(const sf::String &name, Callback callback, void *user);
	static ContentLoadHandle loadMainThread(const sf::String &name, Callback callback, void *user);

	static void runMainThreadCallbacks();

	static void mainThreadCallback(MainThreadCallback *cb);

	template <typename Fn>
	sf_forceinline static void mainThreadCallbackFunc(Fn fn) {
		MainThreadCallbackFn<Fn> cb { fn };
		mainThreadCallback(&cb);
	}

	// Cancel an in-flight load
	static void cancel(ContentLoadHandle handle);

	// Lifecycle
	static void globalInit(bool useWorker);
	static void globalCleanup();
	static void globalUpdate();
};

}
