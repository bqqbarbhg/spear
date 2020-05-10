#include "AssetInfo.h"

#include "sf/Reflection.h"
#include "sp/ContentFile.h"
#include "sp/Json.h"

#include "ext/json_input.h"

namespace cl {

void AnimationInfo::refresh()
{
	sp::ModelProps props;
	props.ignoreGeometry = true;
	modelRef.load(file, props);
}

void ModelInfo::refresh()
{
	sp::ModelProps props;
	props.ignoreAnimations = true;
	modelRef.load(model, props);
	skinRef.load(skin);
}

void TileVariantInfo::refresh()
{
	sp::ModelProps props;
	props.cpuData = true;
	props.ignoreAnimations = true;
	if (model) modelRef.load(model);
	if (shadowModel) shadowModelRef.load(shadowModel);
}

void TileInfo::refresh()
{
}

void AssetInfoBase::loadCallback(void *user, const sp::ContentFile &file)
{
	AssetInfoBase *imp = (AssetInfoBase*)user;

	if (file.data == nullptr) {
		imp->assetFailLoading();
		return;
	}

	jsi_args args = { };
	args.dialect.allow_bare_keys = true;
	args.dialect.allow_comments = true;
	args.dialect.allow_control_in_string = true;
	args.dialect.allow_missing_comma = true;
	args.dialect.allow_trailing_comma = true;
	jsi_value *value = jsi_parse_memory(file.data, file.size, &args);
	if (!value) {
		sf::debugPrint("Failed to parse %s:%u:%u: %s",
			imp->name.data, args.error.line, args.error.column, args.error.description);
		imp->assetFailLoading();
		return;
	}

	sp::readInstJson(value, imp + 1, imp->type);

	jsi_free(value);

	imp->assetFinishLoading();
}

void AssetInfoBase::assetStartLoading()
{
	sp::ContentFile::loadMainThread(name, &AssetInfoBase::loadCallback, this);
}

}

namespace sf {

template<> void initType<cl::AnimationInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::AnimationInfo, name),
		sf_field(cl::AnimationInfo, tags),
		sf_field(cl::AnimationInfo, file),
		sf_field(cl::AnimationInfo, clip),
	};
	sf_struct(t, cl::AnimationInfo, fields);
	t->postSerializeFn = [](void *inst, sf::Type *) {
		((cl::AnimationInfo*)inst)->refresh();
	};
}

template<> void initType<cl::ModelInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::ModelInfo, model),
		sf_field(cl::ModelInfo, skin),
		sf_field(cl::ModelInfo, scale),
		sf_field(cl::ModelInfo, animations),
	};
	sf_struct(t, cl::ModelInfo, fields);
	t->postSerializeFn = [](void *inst, sf::Type *) {
		((cl::ModelInfo*)inst)->refresh();
	};
}

template<> void initType<cl::TileVariantInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::TileVariantInfo, model),
		sf_field(cl::TileVariantInfo, shadowModel),
	};
	sf_struct(t, cl::TileVariantInfo, fields);
	t->postSerializeFn = [](void *inst, sf::Type *) {
		((cl::TileVariantInfo*)inst)->refresh();
	};
}

template<> void initType<cl::TileInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::TileInfo, variants),
	};
	sf_struct(t, cl::TileInfo, fields);
	t->postSerializeFn = [](void *inst, sf::Type *) {
		((cl::TileInfo*)inst)->refresh();
	};
}

}
