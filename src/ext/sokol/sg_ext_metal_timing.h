
#ifdef __cplusplus
extern "C" {
#endif

#define SG_EXT_MAX_FRAMES_IN_FLIGHT 4

void sg_ext_mtl_begin_frame();
void sg_ext_mtl_end_frame();
uint32_t sg_ext_mtl_begin_pass();
void sg_ext_mtl_end_pass();
double sg_ext_mtl_pass_time(uint32_t index);

#ifdef __cplusplus
}
#endif

