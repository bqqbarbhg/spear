#include "ContentFile.h"

#include "ext/sokol/sokol_fetch.h"
#include "sf/Array.h"
#include "sf/Mutex.h"

namespace sp {

static constexpr uint32_t NumBuffers = 4;
static constexpr uint32_t BufferSize = 4 * 1024 * 1024;

struct ContentFileInfo
{
	ContentFile::Callback callback;
	void *user;
};

struct ContentFileQueue
{
	sf::Mutex mutex;
	sf::StringBuf root;
	sf::Array<char> fileBuffers[NumBuffers];
	sf::Array<sfetch_handle_t> cancelledHandles;
};

ContentFileQueue g_queue;

static void fetchCallback(const sfetch_response_t *response)
{
	sfetch_handle_t handle = response->handle;
	ContentFileInfo *info = (ContentFileInfo*)response->user_data;

	// Acquire mutex to check for checking cancellation
	// to guarantee that after `ContentFile::cancel()`
	// returns the callback will never be called.
	{
		sf::MutexGuard mg(g_queue.mutex);
		for (sfetch_handle_t &h : g_queue.cancelledHandles) {
			if (h.id == handle.id) {
				if (response->finished) {
					// If this is the final callback remove from the cancel list
					g_queue.cancelledHandles.removeSwap(&h - g_queue.cancelledHandles.data);
				}
				return;
			}
		}
	}

	if (response->failed) {
		// Failed: Return with empty file
		info->callback(info->user, ContentFile{});
	} else if (response->finished) {
		// Finished: Call callback with actual data
		ContentFile cf;
		cf.data = response->buffer_ptr;
		cf.size = response->fetched_size;
		info->callback(info->user, cf);
	} else if (response->dispatched) {
		// Dispatched: Allocate a buffer
		void *ptr = g_queue.fileBuffers[response->lane].data;
		sfetch_bind_buffer(handle, ptr, BufferSize);
	}
}

void ContentFile::init(const sf::String &root)
{
	g_queue.root = root;
	for (auto &buffer : g_queue.fileBuffers) {
		buffer.resizeUninit(BufferSize);
	}

	sfetch_desc_t desc = { };
	desc.num_lanes = NumBuffers;
	sfetch_setup(&desc);
}

void ContentFile::update()
{
	sfetch_dowork();
}

ContentFile::LoadHandle ContentFile::load(const sf::String &name, Callback callback, void *user)
{
	ContentFileInfo info;
	info.callback = callback;
	info.user = user;

	sf::SmallStringBuf<256> path;
	path.append(g_queue.root, name);

	sfetch_request_t req = { };
	req.callback = &fetchCallback;
	req.path = path.data;
	req.user_data_ptr = &info;
	req.user_data_size = sizeof(ContentFileInfo);
	sfetch_handle_t handle = sfetch_send(&req);

	LoadHandle lh;
	lh.id = handle.id;
	return lh;
}

void ContentFile::cancel(LoadHandle handle)
{
	sf::MutexGuard mg(g_queue.mutex);
	sfetch_handle_t h = { handle.id };
	g_queue.cancelledHandles.push(h);
	sfetch_cancel(h);
}

}
