
#ifndef SP_NORMALMAP_REMAP
    #error "Permutation SP_NORMALMAP_REMAP not defined"
#endif

#if SP_NORMALMAP_REMAP

#define sampleNormalMap(tex, uv) (texture(tex, uv).yw * 2.0 - asVec2(1.0))

#else

#define sampleNormalMap(tex, uv) (texture(tex, uv).xy * 2.0 - asVec2(1.0))

#endif
