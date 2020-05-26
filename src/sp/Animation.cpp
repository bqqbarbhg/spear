#include "Animation.h"
#include "ext/acl/decompression/decompress.h"
#include "ext/acl/algorithm/uniformly_sampled/decoder.h"
#include "ext/acl/decompression/output_writer.h"

#include "sf/Array.h"
#include "ContentFile.h"
#include "ext/sp_tools_common.h"

namespace sp {

struct AnimationImp : Animation
{
	sf::Array<char> data;
	acl::uniformly_sampled::DecompressionContext<acl::uniformly_sampled::DefaultDecompressionSettings> decompressionContext;

	virtual void assetStartLoading() final;
	virtual void assetUnload() final;
};

AssetType Animation::SelfType = { "Animation", sizeof(AnimationImp), sizeof(Animation::PropType),
	[](Asset *a) { new ((AnimationImp*)a) AnimationImp(); }
};

static sf::Symbol toSymbol(char *strings, spfile_string str)
{
	return sf::Symbol(strings + str.offset, str.length);
}

static void loadImp(void *user, const ContentFile &file)
{
	AnimationImp *imp = (AnimationImp*)user;

	spanim_util su;
	spanim_util_init(&su, file.data, file.size);
	spanim_header header = spanim_decode_header(&su);
	char *strings = spanim_decode_strings(&su);
	spanim_bone *bones = spanim_decode_bones(&su);
	if (!spfile_util_failed(&su.file)) {
		imp->data.resizeUninit(header.s_animation.uncompressed_size);
		spanim_decode_animation_to(&su, imp->data.data);
	}

	if (spfile_util_failed(&su.file)) {
		imp->assetFailLoading();
		spfile_util_free(&su.file);
		return;
	}

	imp->duration = (float)header.info.duration;

	imp->bones.reserve(header.info.num_bones);
	for (uint32_t i = 0; i < header.info.num_bones; i++) {
		spanim_bone &sp_bone = bones[i];
		AnimationBone &bone = imp->bones.push();
		bone.parentIx = sp_bone.parent;
		bone.name = toSymbol(strings, sp_bone.name);
	}

	imp->decompressionContext.initialize(*(acl::CompressedClip*)imp->data.data);

	spfile_util_free(&su.file);
	imp->assetFinishLoading();
}

void AnimationImp::assetStartLoading()
{
	sf::SmallStringBuf<256> assetName;
	assetName.append(name, ".spanim");

	ContentFile::loadAsync(assetName, &loadImp, this);
}

void AnimationImp::assetUnload()
{
}

struct AclOutputWriter final : public acl::OutputWriter
{
	sf::Slice<const uint32_t> boneMapping;
	sf::Slice<BoneTransform> transforms;

	void RTM_SIMD_CALL write_bone_rotation(uint16_t bone_index, rtm::quatf_arg0 rotation)
	{
		uint32_t ix = boneMapping[bone_index];
		if (ix != ~0u) {
			rtm::quat_store(rotation, transforms[ix].rotation.v);
		}
	}

	void RTM_SIMD_CALL write_bone_translation(uint16_t bone_index, rtm::vector4f_arg0 translation)
	{
		uint32_t ix = boneMapping[bone_index];
		if (ix != ~0u) {
			rtm::vector_store3(translation, transforms[ix].translation.v);
		}
	}

	void RTM_SIMD_CALL write_bone_scale(uint16_t bone_index, rtm::vector4f_arg0 scale)
	{
		uint32_t ix = boneMapping[bone_index];
		if (ix != ~0u) {
			rtm::vector_store3(scale, transforms[ix].scale.v);
		}
	}
};

void Animation::generateBoneMapping(Model *model, sf::Slice<uint32_t> dst)
{
	sf_assert(model->isLoaded() && isLoaded());
	sf_assert(dst.size >= bones.size);

	uint32_t *ptr = dst.data;
	for (AnimationBone &animBone : bones) {
		auto it = model->boneNames.find(animBone.name);
		if (it) {
			*ptr++ = it->val;
		} else {
			*ptr++ = ~0u;
		}
	}
}

void Animation::evaluate(float time, sf::Slice<const uint32_t> boneMapping, sf::Slice<BoneTransform> transforms)
{
	AnimationImp *imp = (AnimationImp*)this;
	imp->decompressionContext.seek(time, acl::sample_rounding_policy::none);

	AclOutputWriter writer;
	writer.boneMapping = boneMapping;
	writer.transforms = transforms;
	imp->decompressionContext.decompress_pose(writer);
}

sf::Mat34 boneTransformToMatrix(const BoneTransform &t)
{
	return sf::mat::world(t.translation, t.rotation, t.scale);
}

void boneTransformToWorld(Model *model, sf::Slice<sf::Mat34> dst, const sf::Slice<BoneTransform> src, const sf::Mat34 &toWorld)
{
	if (!model) return;
	uint32_t numBones = model->bones.size;
	if (numBones == 0) return;

	dst[0] = toWorld * boneTransformToMatrix(src[0]);
	for (uint32_t boneI = 1; boneI < numBones; boneI++) {
		uint32_t parentI = model->bones[boneI].parentIx;
		dst[boneI] = dst[parentI] * boneTransformToMatrix(src[boneI]);
	}
}

}
