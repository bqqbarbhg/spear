#include "ServerState.h"
#include "sf/Reflection.h"

namespace sf {
using namespace sv;

template<> void initType<Component>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(Component, Model, ModelComponent),
		sf_poly(Component, PointLight, PointLightComponent),
		sf_poly(Component, ParticleSystem, ParticleSystemComponent),
		sf_poly(Component, Character, CharacterComponent),
		sf_poly(Component, CharacterModel, CharacterModelComponent),
		sf_poly(Component, BlobShadow, BlobShadowComponent),
		sf_poly(Component, Card, CardComponent),
		sf_poly(Component, CardAttach, CardAttachComponent),
		sf_poly(Component, CardMelee, CardMeleeComponent),
		sf_poly(Component, CardProjectile, CardProjectileComponent),
		sf_poly(Component, CastOnTurnStart, CastOnTurnStartComponent),
		sf_poly(Component, CastOnReceiveDamage, CastOnReceiveDamageComponent),
		sf_poly(Component, CastOnDealDamage, CastOnDealDamageComponent),
		sf_poly(Component, CardCast, CardCastComponent),
		sf_poly(Component, Spell, SpellComponent),
		sf_poly(Component, SpellDamage, SpellDamageComponent),
		sf_poly(Component, SpellStatus, SpellStatusComponent),
		sf_poly(Component, Status, StatusComponent),
		sf_poly(Component, CharacterTemplate, CharacterTemplateComponent),
	};
	sf_struct_poly(t, Component, type, { }, polys);
}

template<> void initType<Event>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(Event, CardCooldownTick, CardCooldownTickEvent),
		sf_poly(Event, StatusAdd, StatusAddEvent),
		sf_poly(Event, StatusTick, StatusTickEvent),
		sf_poly(Event, StatusRemove, StatusRemoveEvent),
		sf_poly(Event, CastSpell, CastSpellEvent),
		sf_poly(Event, Damage, DamageEvent),
		sf_poly(Event, LoadPrefab, LoadPrefabEvent),
		sf_poly(Event, RemoveGarbageIds, RemoveGarbageIdsEvent),
		sf_poly(Event, RemoveGarbagePrefabs, RemoveGarbagePrefabsEvent),
		sf_poly(Event, AddProp, AddPropEvent),
		sf_poly(Event, AddCharacter, AddCharacterEvent),
		sf_poly(Event, AddCard, AddCardEvent),
		sf_poly(Event, GiveCard, GiveCardEvent),
		sf_poly(Event, SelectCard, SelectCardEvent),
		sf_poly(Event, AddCharacterToSpawn, AddCharacterToSpawn),
		sf_poly(Event, SelectCharacterToSpawn, SelectCharacterToSpawnEvent),
	};
	sf_struct_poly(t, Event, type, { }, polys);
}

template<> void initType<ModelComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(ModelComponent, model),
		sf_field(ModelComponent, shadowModel),
		sf_field(ModelComponent, material),
		sf_field(ModelComponent, position),
		sf_field(ModelComponent, rotation),
		sf_field(ModelComponent, scale),
		sf_field(ModelComponent, stretch),
		sf_field(ModelComponent, tintColor),
		sf_field(ModelComponent, castShadows),
	};
	sf_struct_base(t, ModelComponent, Component, fields);
}

template<> void initType<PointLightComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(PointLightComponent, color),
		sf_field(PointLightComponent, intensity),
		sf_field(PointLightComponent, radius),
		sf_field(PointLightComponent, position),
		sf_field(PointLightComponent, flickerFrequency),
		sf_field(PointLightComponent, flickerIntensity),
	};
	sf_struct_base(t, PointLightComponent, Component, fields);
}

template<> void initType<ParticleSystemComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(ParticleSystemComponent, sprite),
		sf_field(ParticleSystemComponent, color),
		sf_field(ParticleSystemComponent, intensity),
		sf_field(ParticleSystemComponent, radius),
		sf_field(ParticleSystemComponent, position),
	};
	sf_struct_base(t, ParticleSystemComponent, Component, fields);
}

template<> void initType<CharacterComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CharacterComponent, name),
		sf_field(CharacterComponent, description),
		sf_field(CharacterComponent, image),
		sf_field(CharacterComponent, maxHealth),
		sf_field(CharacterComponent, baseArmor),
		sf_field(CharacterComponent, minWeightDice),
		sf_field(CharacterComponent, maxWeightDice),
		sf_field(CharacterComponent, skillSlots),
		sf_field(CharacterComponent, spellSlots),
		sf_field(CharacterComponent, itemSlots),
	};
	sf_struct_base(t, CharacterComponent, Component, fields);
}

template<> void initType<CharacterModelComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CharacterModelComponent, modelName),
		sf_field(CharacterModelComponent, scale),
		sf_field(CharacterModelComponent, animations),
	};
	sf_struct_base(t, CharacterModelComponent, Component, fields);
}

template<> void initType<BlobShadowComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(BlobShadowComponent, blobs),
	};
	sf_struct_base(t, BlobShadowComponent, Component, fields);
}

template<> void initType<CardComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CardComponent, name),
		sf_field(CardComponent, description),
		sf_field(CardComponent, cooldown),
		sf_field(CardComponent, melee),
		sf_field(CardComponent, skill),
		sf_field(CardComponent, spell),
		sf_field(CardComponent, item),
	};
	sf_struct_base(t, CardComponent, Component, fields);
}

template<> void initType<CardAttachComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CardAttachComponent, prefabName),
		sf_field(CardAttachComponent, boneName),
	};
	sf_struct_base(t, CardAttachComponent, Component, fields);
}

template<> void initType<CardMeleeComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CardMeleeComponent, hitRoll),
	};
	sf_struct_base(t, CardMeleeComponent, Component, fields);
}

template<> void initType<CardProjectileComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CardProjectileComponent, prefabName),
		sf_field(CardProjectileComponent, flightSpeed),
	};
	sf_struct_base(t, CardProjectileComponent, Component, fields);
}

template<> void initType<CastOnTurnStartComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CastOnTurnStartComponent, spellName),
	};
	sf_struct_base(t, CastOnTurnStartComponent, Component, fields);
}

template<> void initType<CastOnReceiveDamageComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CastOnReceiveDamageComponent, onMelee),
		sf_field(CastOnReceiveDamageComponent, onSpell),
		sf_field(CastOnReceiveDamageComponent, spellName),
	};
	sf_struct_base(t, CastOnReceiveDamageComponent, Component, fields);
}

template<> void initType<CastOnDealDamageComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CastOnDealDamageComponent, onMelee),
		sf_field(CastOnDealDamageComponent, onSpell),
		sf_field(CastOnDealDamageComponent, spellName),
	};
	sf_struct_base(t, CastOnDealDamageComponent, Component, fields);
}

template<> void initType<CardCastComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CardCastComponent, spellName),
	};
	sf_struct_base(t, CardCastComponent, Component, fields);
}

template<> void initType<SpellComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(SpellComponent, successRoll),
	};
	sf_struct_base(t, SpellComponent, Component, fields);
}

template<> void initType<SpellDamageComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(SpellDamageComponent, damageRoll),
	};
	sf_struct_base(t, SpellDamageComponent, Component, fields);
}

template<> void initType<SpellStatusComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(SpellStatusComponent, statusName),
	};
	sf_struct_base(t, SpellStatusComponent, Component, fields);
}

template<> void initType<StatusComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(StatusComponent, turnsRoll),
	};
	sf_struct_base(t, StatusComponent, Component, fields);
}

template<> void initType<CharacterTemplateComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(CharacterTemplateComponent, name),
		sf_field(CharacterTemplateComponent, description),
		sf_field(CharacterTemplateComponent, characterPrefab),
		sf_field(CharacterTemplateComponent, starterCardPrefabs),
	};
	sf_struct_base(t, CharacterTemplateComponent, Component, fields);
}

template<> void initType<CardCooldownTickEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(CardCooldownTickEvent, cardId),
	};
	sf_struct_base(t, CardCooldownTickEvent, Event, fields);
}

template<> void initType<StatusAddEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(StatusAddEvent, turnsRoll),
		sf_field(StatusAddEvent, status),
	};
	sf_struct_base(t, StatusAddEvent, Event, fields);
}

template<> void initType<StatusTickEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(StatusTickEvent, statusId),
	};
	sf_struct_base(t, StatusTickEvent, Event, fields);
}

template<> void initType<StatusRemoveEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(StatusRemoveEvent, statusId),
	};
	sf_struct_base(t, StatusRemoveEvent, Event, fields);
}

template<> void initType<CastSpellEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(CastSpellEvent, spellInfo),
		sf_field(CastSpellEvent, successRoll),
	};
	sf_struct_base(t, CastSpellEvent, Event, fields);
}

template<> void initType<DamageEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(DamageEvent, damageInfo),
		sf_field(DamageEvent, damageRoll),
		sf_field(DamageEvent, finalDamage),
		sf_field(DamageEvent, meleeArmor),
	};
	sf_struct_base(t, DamageEvent, Event, fields);
}

template<> void initType<LoadPrefabEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(LoadPrefabEvent, prefab),
	};
	sf_struct_base(t, LoadPrefabEvent, Event, fields);
}

template<> void initType<RemoveGarbageIdsEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(RemoveGarbageIdsEvent, ids),
	};
	sf_struct_base(t, RemoveGarbageIdsEvent, Event, fields);
}

template<> void initType<RemoveGarbagePrefabsEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(RemoveGarbagePrefabsEvent, names),
	};
	sf_struct_base(t, RemoveGarbagePrefabsEvent, Event, fields);
}

template<> void initType<AddPropEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(AddPropEvent, prop),
	};
	sf_struct_base(t, AddPropEvent, Event, fields);
}

template<> void initType<AddCharacterEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(AddCharacterEvent, character),
	};
	sf_struct_base(t, AddCharacterEvent, Event, fields);
}

template<> void initType<AddCardEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(AddCardEvent, card),
	};
	sf_struct_base(t, AddCardEvent, Event, fields);
}

template<> void initType<GiveCardEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(GiveCardEvent, cardId),
		sf_field(GiveCardEvent, previousOwnerId),
		sf_field(GiveCardEvent, ownerId),
	};
	sf_struct_base(t, GiveCardEvent, Event, fields);
}

template<> void initType<SelectCardEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(SelectCardEvent, ownerId),
		sf_field(SelectCardEvent, cardId),
		sf_field(SelectCardEvent, slot),
	};
	sf_struct_base(t, SelectCardEvent, Event, fields);
}

template<> void initType<AddCharacterToSpawn>(Type *t)
{
	static Field fields[] = {
		sf_field(AddCharacterToSpawn, selectPrefab),
		sf_field(AddCharacterToSpawn, count),
	};
	sf_struct_base(t, AddCharacterToSpawn, Event, fields);
}

template<> void initType<SelectCharacterToSpawnEvent>(Type *t)
{
	static Field fields[] = {
		sf_field(SelectCharacterToSpawnEvent, selectPrefab),
		sf_field(SelectCharacterToSpawnEvent, playerId),
	};
	sf_struct_base(t, SelectCharacterToSpawnEvent, Event, fields);
}

template<> void initType<DiceRoll>(Type *t)
{
	static Field fields[] = {
		sf_field(DiceRoll, num),
		sf_field(DiceRoll, die),
		sf_field(DiceRoll, bias),
		sf_field(DiceRoll, check),
	};
	sf_struct(t, DiceRoll, fields);
}

template<> void initType<AnimationInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(AnimationInfo, name),
		sf_field(AnimationInfo, tags),
		sf_field(AnimationInfo, file),
	};
	sf_struct(t, AnimationInfo, fields);
}

template<> void initType<ShadowBlob>(Type *t)
{
	static Field fields[] = {
		sf_field(ShadowBlob, boneName),
		sf_field(ShadowBlob, radius),
		sf_field(ShadowBlob, alpha),
		sf_field(ShadowBlob, offset),
	};
	sf_struct(t, ShadowBlob, fields);
}

template<> void initType<Prefab>(Type *t)
{
	static Field fields[] = {
		sf_field(Prefab, name),
		sf_field(Prefab, components),
	};
	sf_struct(t, Prefab, fields);
}

template<> void initType<Prop>(Type *t)
{
	static Field fields[] = {
		sf_field(Prop, id),
		sf_field(Prop, prefabName),
		sf_field(Prop, tile),
		sf_field(Prop, offset),
		sf_field(Prop, rotation),
		sf_field(Prop, scale),
		sf_field(Prop, deleted),
	};
	sf_struct(t, Prop, fields);
}

template<> void initType<Card>(Type *t)
{
	static Field fields[] = {
		sf_field(Card, id),
		sf_field(Card, ownerId),
		sf_field(Card, prefabName),
		sf_field(Card, cooldownLeft),
		sf_field(Card, deleted),
	};
	sf_struct(t, Card, fields);
}

template<> void initType<Status>(Type *t)
{
	static Field fields[] = {
		sf_field(Status, id),
		sf_field(Status, characterId),
		sf_field(Status, prefabName),
		sf_field(Status, cardName),
		sf_field(Status, originalCasterId),
		sf_field(Status, casterId),
		sf_field(Status, turnsLeft),
		sf_field(Status, deleted),
	};
	sf_struct(t, Status, fields);
}

template<> void initType<Character>(Type *t)
{
	static Field fields[] = {
		sf_field(Character, id),
		sf_field(Character, maxHealth),
		sf_field(Character, health),
		sf_field(Character, prefabName),
		sf_field(Character, selectedCards),
		sf_field(Character, cards),
		sf_field(Character, statuses),
		sf_field(Character, tile),
		sf_field(Character, armor),
		sf_field(Character, deleted),
	};
	sf_struct(t, Character, fields);
}

template<> void initType<StatusInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(StatusInfo, originalCasterId),
		sf_field(StatusInfo, casterId),
		sf_field(StatusInfo, targetId),
		sf_field(StatusInfo, statusName),
		sf_field(StatusInfo, cardName),
	};
	sf_struct(t, StatusInfo, fields);
}

template<> void initType<SpellInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(SpellInfo, originalCasterId),
		sf_field(SpellInfo, casterId),
		sf_field(SpellInfo, targetId),
		sf_field(SpellInfo, spellName),
		sf_field(SpellInfo, cardName),
	};
	sf_struct(t, SpellInfo, fields);
}

template<> void initType<MeleeInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(MeleeInfo, attackerId),
		sf_field(MeleeInfo, targetId),
		sf_field(MeleeInfo, cardName),
	};
	sf_struct(t, MeleeInfo, fields);
}

template<> void initType<DamageInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(DamageInfo, melee),
		sf_field(DamageInfo, physical),
		sf_field(DamageInfo, magic),
		sf_field(DamageInfo, spellName),
		sf_field(DamageInfo, originalCasterId),
		sf_field(DamageInfo, causeId),
		sf_field(DamageInfo, targetId),
		sf_field(DamageInfo, damageRoll),
	};
	sf_struct(t, DamageInfo, fields);
}

template<> void initType<RollInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(RollInfo, name),
		sf_field(RollInfo, roll),
		sf_field(RollInfo, total),
	};
	sf_struct(t, RollInfo, fields);
}

template<> void initType<CastInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(CastInfo, spellInfo),
		sf_field(CastInfo, rolls),
		sf_field(CastInfo, succeeded),
	};
	sf_struct(t, CastInfo, fields);
}

}

