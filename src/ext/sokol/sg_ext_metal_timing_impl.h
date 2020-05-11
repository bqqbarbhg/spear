#include "sg_ext_metal_timing.h"

#define SG_EXT_MAX_PASSES_PER_FRAME 1024

static id<MTLCommandBuffer> _sg_ext_buffer_queue[SG_EXT_MAX_PASSES_PER_FRAME * SG_EXT_MAX_FRAMES_IN_FLIGHT];
static double _sg_ext_pass_times[SG_EXT_MAX_PASSES_PER_FRAME * SG_EXT_MAX_FRAMES_IN_FLIGHT];
static uint32_t _sg_ext_buffer_index = 0;
static uint32_t _sg_ext_buffer_frame_index = 0;

void sg_ext_mtl_begin_frame()
{
	dispatch_semaphore_wait(_sg_mtl_sem, DISPATCH_TIME_FOREVER);
	_sg_ext_buffer_frame_index = (_sg_ext_buffer_frame_index + 1) % SG_EXT_MAX_FRAMES_IN_FLIGHT;
	uint32_t ix = _sg_ext_buffer_frame_index * SG_EXT_MAX_PASSES_PER_FRAME;
	_sg_ext_buffer_index = ix;
	for (; nil != _sg_ext_buffer_queue[ix]; ix++) {
        double start = [_sg_ext_buffer_queue[ix] GPUStartTime];
        double end = [_sg_ext_buffer_queue[ix] GPUEndTime];

        _sg_ext_pass_times[ix] = end - start;
		_sg_ext_buffer_queue[ix] = nil;
	}
}

void sg_ext_mtl_end_frame()
{
	if (nil == _sg_mtl_cmd_buffer) {
		_sg_mtl_cmd_buffer = [_sg_mtl_cmd_queue commandBufferWithUnretainedReferences];
	}
}

uint32_t sg_ext_mtl_begin_pass(const char *name)
{
	if (nil != _sg_mtl_cmd_buffer) {
		[_sg_mtl_cmd_buffer commit];
	}

	_sg_mtl_cmd_buffer = [_sg_mtl_cmd_queue commandBufferWithUnretainedReferences];
    // [_sg_mtl_cmd_buffer pushDebugGroup: [NSString stringWithUTF8String: name]];

	uint32_t index = _sg_ext_buffer_index++;
	_sg_ext_buffer_queue[index] = _sg_mtl_cmd_buffer;
    if (name) _sg_mtl_cmd_buffer.label = [NSString stringWithUTF8String: name];
	return index;
}

void sg_ext_mtl_end_pass()
{
    SOKOL_ASSERT(nil != _sg_mtl_cmd_buffer);
    // [_sg_mtl_cmd_buffer popDebugGroup];
	[_sg_mtl_cmd_buffer commit];
	_sg_mtl_cmd_buffer = nil;
}

double sg_ext_mtl_pass_time(uint32_t index)
{
	return _sg_ext_pass_times[index];
}

