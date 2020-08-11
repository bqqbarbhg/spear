#pragma once

#include "sf/HashMap.h"
#include "sf/HashSet.h"
#include "sf/ImplicitHashMap.h"
#include "sf/UintMap.h"
#include "sf/UintSet.h"
#include "sf/Box.h"
#include "sf/Symbol.h"
#include "sf/String.h"
#include "sf/Vector.h"
#include "sf/Array.h"

#define sv_reflect

namespace sv {

struct DiceRoll sv_reflect
{
	uint32_t num;
	uint32_t die;
	uint32_t bias;
	uint32_t check;
};

struct Component
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		Model,
		PointLight,
		ParticleSystem,
		Character,
		CharacterModel,
		BlobShadow,
		Card,
		CardAttach,
		CardMelee,
		CardProjectile,
		CastOnTurnStart,
		CastOnReceiveDamage,
		CastOnDealDamage,
		ResistDamage,
		CardCast,
		Spell,
		SpellDamage,
		SpellStatus,
		Status,
		CharacterTemplate,
		TileArea,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Component() { }
	Component(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::ComponentType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::ComponentType ? (T*)this : nullptr; }
};

template <Component::Type SelfType>
struct ComponentBase : Component
{
	static constexpr Type ComponentType = SelfType;
	ComponentBase() : Component(SelfType) { }
};

struct ModelComponent : ComponentBase<Component::Model>
{
	sf::Symbol model;
	sf::Symbol shadowModel;
	sf::Symbol material;
	sf::Vec3 position;
	sf::Vec3 rotation;
	float scale = 1.0f;
	sf::Vec3 stretch = sf::Vec3(1.0f);
	uint8_t tintColor[3] = { 255, 255, 255 };
	bool castShadows = true;
};

struct PointLightComponent : ComponentBase<Component::PointLight>
{
	sf::Vec3 color = sf::Vec3(1.0f);
	float intensity = 1.0f;
	float radius = 1.0f;
	sf::Vec3 position;
	float flickerFrequency = 0.0f;
	float flickerIntensity = 0.0f;
	bool castShadows = true;
};

struct ParticleSystemComponent : ComponentBase<Component::ParticleSystem>
{
	sf::Symbol sprite;
	sf::Vec3 color = sf::Vec3(1.0f);
	float intensity = 1.0f;
	float radius = 1.0f;
	sf::Vec3 position;
	bool updateOutOfCamera = false;
	float prewarmTime = 0.0f;
};

struct CharacterComponent : ComponentBase<Component::Character>
{
	sf::Symbol name;
	sf::Symbol description;
	sf::Symbol image;
	uint32_t maxHealth = 0;
	uint32_t baseArmor = 0;
	uint32_t minWeightDice = 1;
	uint32_t maxWeightDice = 100;
	uint32_t skillSlots = 0;
	uint32_t spellSlots = 0;
	uint32_t itemSlots = 0;
};

struct AnimationInfo sv_reflect
{
	sf::Symbol name;
	sf::Array<sf::Symbol> tags;
	sf::Symbol file;
};

struct CharacterModelComponent : ComponentBase<Component::CharacterModel>
{
	sf::Symbol modelName;
	sf::HashMap<sf::Symbol, sf::Symbol> materials;
	float scale = 1.0f;
	sf::Array<AnimationInfo> animations;
};

struct ShadowBlob sv_reflect
{
	sf::Symbol boneName;
	float radius = 0.0f;
	float alpha = 0.0f;
	sf::Vec3 offset;
};

struct BlobShadowComponent : ComponentBase<Component::BlobShadow>
{
	sf::Array<ShadowBlob> blobs;
};

struct CardComponent : ComponentBase<Component::Card>
{
	sf::Symbol name;
	sf::Symbol description;
	uint32_t cooldown;
	// TODO: Enum
	bool melee = false;
	bool skill = false;
	bool spell = false;
	bool item = false;
};

struct CardAttachComponent : ComponentBase<Component::CardAttach>
{
	sf::Symbol prefabName;
	sf::Symbol boneName;
};

struct CardMeleeComponent : ComponentBase<Component::CardMelee>
{
	DiceRoll hitRoll;
};

struct CardProjectileComponent : ComponentBase<Component::CardProjectile>
{
	sf::Symbol prefabName;
	sf::Symbol hitEffect;
	float flightSpeed = 0.0f;
};

struct CastOnTurnStartComponent : ComponentBase<Component::CastOnTurnStart>
{
	sf::Symbol spellName;
};

struct CastOnReceiveDamageComponent : ComponentBase<Component::CastOnReceiveDamage>
{
	bool onMelee = false;
	bool onSpell = false;
	sf::Symbol spellName;
};

struct CastOnDealDamageComponent : ComponentBase<Component::CastOnDealDamage>
{
	bool onMelee = false;
	bool onSpell = false;
	sf::Symbol spellName;
};

struct ResistDamageComponent : ComponentBase<Component::ResistDamage>
{
	// TODO: Enum
	bool onSpell = false;
	bool onMelee = false;
	float resistAmount = 1.0f;
	DiceRoll successRoll;
	sf::Symbol effectName;
};

struct CardCastComponent : ComponentBase<Component::CardCast>
{
	sf::Symbol spellName;
};

struct SpellComponent : ComponentBase<Component::Spell>
{
	DiceRoll successRoll;
};

struct SpellDamageComponent : ComponentBase<Component::SpellDamage>
{
	DiceRoll damageRoll;
};

struct SpellStatusComponent : ComponentBase<Component::SpellStatus>
{
	sf::Symbol statusName;
};

struct StatusComponent : ComponentBase<Component::Status>
{
	DiceRoll turnsRoll;
	sf::Symbol startEffect;
	sf::Symbol activeEffect;
	sf::Symbol endEffect;
};

struct CharacterTemplateComponent : ComponentBase<Component::CharacterTemplate>
{
	sf::Symbol name;
	sf::Symbol description;
	sf::Symbol characterPrefab;
	sf::Array<sf::Symbol> starterCardPrefabs;
};

struct TileAreaComponent : ComponentBase<Component::TileArea>
{
	sf::Vec2i minCorner;
	sf::Vec2i maxCorner;
};

struct Prefab sv_reflect
{
	sf::Symbol name;
	sf::Array<sf::Box<Component>> components;

	Component *findComponentImp(Component::Type type) const;

	template <typename T>
	T *findComponent() const { return (T*)findComponentImp(T::ComponentType); }
};

struct PropTransform sv_reflect
{
	sf::Vec2i position;     // 1/2^16 m
	int32_t offsetY = 0;    // 1/2^16 m
	uint16_t rotation = 0;  // 1/2^6 deg
	uint16_t scale = 0x100; // 1/2^8 x
};

struct Prop sv_reflect
{
	uint32_t id;
	PropTransform transform;
	sf::Symbol prefabName;
};

struct Card sv_reflect
{
	uint32_t id;
	uint32_t ownerId = 0;
	sf::Symbol prefabName;
	uint32_t cooldownLeft;
};

struct Status sv_reflect
{
	uint32_t id;
	uint32_t characterId;
	sf::Symbol prefabName;
	sf::Symbol cardName;
	uint32_t originalCasterId;
	uint32_t casterId;
	uint32_t turnsLeft;
};

static const uint32_t NumSelectedCards = 8;

struct Character sv_reflect
{
	uint32_t id;
	int32_t maxHealth = 0;
	int32_t health = 0;
	sf::Symbol prefabName;
	uint32_t selectedCards[NumSelectedCards] = { };
	sf::Array<uint32_t> cards;
	sf::Array<uint32_t> statuses;
	sf::Vec2i tile;
	uint32_t armor = 0;
};

struct StatusInfo sv_reflect
{
	uint32_t originalCasterId;
	uint32_t casterId;
	uint32_t targetId;
	sf::Symbol statusName;
	sf::Symbol cardName;
};

struct SpellInfo sv_reflect
{
	uint32_t originalCasterId;
	uint32_t casterId;
	uint32_t targetId;
	sf::Symbol spellName;
	sf::Symbol cardName;
};

struct MeleeInfo sv_reflect
{
	uint32_t attackerId;
	uint32_t targetId;
	sf::Symbol cardName;
};

struct DamageInfo sv_reflect
{
	bool melee = false;
	bool physical = false;
	bool magic = false;
	sf::Symbol spellName;
	uint32_t originalCasterId;
	uint32_t causeId;
	uint32_t targetId;
	DiceRoll damageRoll;
};

struct RollInfo sv_reflect
{
	sf::Symbol name;
	DiceRoll roll;
	sf::SmallArray<uint32_t, 2> results;
	uint32_t total;
};

struct CastInfo sv_reflect
{
	SpellInfo spellInfo;
	sf::Array<RollInfo> rolls;
	bool succeeded = false;
};

struct Event
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		AllocateId,
		CardCooldownTick,
		StatusAdd,
		StatusTick,
		StatusRemove,
		ResistDamage,
		CastSpell,
		MeleeAttack,
		Damage,
		LoadPrefab,
		ReloadPrefab,
		MakeUniquePrefab,
		RemoveGarbageIds,
		RemoveGarbagePrefabs,
		AddProp,
		RemoveProp,
		ReplaceLocalProp,
		AddCharacter,
		AddCard,
		MoveProp,
		GiveCard,
		SelectCard,
		AddCharacterToSpawn,
		SelectCharacterToSpawn,
		Move,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Event() { }
	Event(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::EventType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::EventType ? (T*)this : nullptr; }
};

template <Event::Type SelfType>
struct EventBase : Event
{
	static constexpr Type EventType = SelfType;
	EventBase() : Event(SelfType) { }
};

struct AllocateIdEvent : EventBase<Event::AllocateId>
{
	uint32_t id;
};

struct CardCooldownTickEvent : EventBase<Event::CardCooldownTick>
{
	uint32_t cardId;
};

struct StatusAddEvent : EventBase<Event::StatusAdd>
{
	RollInfo turnsRoll;
	Status status;
};

struct StatusTickEvent : EventBase<Event::StatusTick>
{
	uint32_t statusId;
};

struct StatusRemoveEvent : EventBase<Event::StatusRemove>
{
	uint32_t statusId;
};

struct ResistDamageEvent : EventBase<Event::ResistDamage>
{
	sf::Symbol cardName;
	sf::Symbol effectName;
	float resistAmount;
	uint32_t resistDamage;
	RollInfo successRoll;
	bool success;
};

struct CastSpellEvent : EventBase<Event::CastSpell>
{
	SpellInfo spellInfo;
	RollInfo successRoll;
};

struct MeleeAttackEvent : EventBase<Event::CastSpell>
{
	MeleeInfo meleeInfo;
	DiceRoll hitRoll;
};

struct DamageEvent : EventBase<Event::Damage>
{
	DamageInfo damageInfo;
	RollInfo damageRoll;
	uint32_t finalDamage = 0;
	uint32_t meleeArmor = 0;
};

struct LoadPrefabEvent : EventBase<Event::LoadPrefab>
{
	Prefab prefab;
};

struct ReloadPrefabEvent : EventBase<Event::ReloadPrefab>
{
	Prefab prefab;
};

struct MakeUniquePrefabEvent : EventBase<Event::MakeUniquePrefab>
{
	uint32_t clientId;
	sf::Symbol prefabName;
	sf::Symbol uniquePrefabName;
	sf::Array<uint32_t> propIds;
};

struct RemoveGarbageIdsEvent : EventBase<Event::RemoveGarbageIds>
{
	sf::Array<uint32_t> ids;
};

struct RemoveGarbagePrefabsEvent : EventBase<Event::RemoveGarbagePrefabs>
{
	sf::Array<sf::Symbol> names;
};

struct AddPropEvent : EventBase<Event::AddProp>
{
	Prop prop;
};

struct RemovePropEvent : EventBase<Event::RemoveProp>
{
	uint32_t propId;
};

struct ReplaceLocalPropEvent : EventBase<Event::ReplaceLocalProp>
{
	uint32_t clientId;
	uint32_t localId;
	Prop prop;
};

struct AddCharacterEvent : EventBase<Event::AddCharacter>
{
	Character character;
};

struct AddCardEvent : EventBase<Event::AddCard>
{
	Card card;
};

struct MovePropEvent : EventBase<Event::MoveProp>
{
	uint32_t propId;
	PropTransform transform;
};

struct GiveCardEvent : EventBase<Event::GiveCard>
{
	uint32_t cardId;
	uint32_t previousOwnerId;
	uint32_t ownerId;
};

struct SelectCardEvent : EventBase<Event::SelectCard>
{
	uint32_t ownerId;
	uint32_t cardId;
	uint32_t slot;
};

struct AddCharacterToSpawn : EventBase<Event::AddCharacterToSpawn>
{
	sf::Symbol selectPrefab;
	int32_t count;
};

struct SelectCharacterToSpawnEvent : EventBase<Event::SelectCharacterToSpawn>
{
	sf::Symbol selectPrefab;
	uint32_t playerId;
};

struct MoveEvent : EventBase<Event::Move>
{
	uint32_t characterId;
	sf::Vec2i position;
};

enum class IdType {
	Null,
	Prop,
	Character,
	Card,
	Status,
	ClientStart,
};

struct Edit
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		PreloadPrefab,
		ModifyPrefab,
		MakeUniquePrefab,
		AddProp,
		CloneProp,
		MoveProp,
		RemoveProp,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Edit() { }
	Edit(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::EditType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::EditType ? (T*)this : nullptr; }
};

template <Edit::Type SelfType>
struct EditBase : Edit
{
	static constexpr Type EditType = SelfType;
	EditBase() : Edit(SelfType) { }
};

struct PreloadPrefabEdit : EditBase<Edit::PreloadPrefab>
{
	sf::Symbol prefabName;
};

struct ModifyPrefabEdit : EditBase<Edit::ModifyPrefab>
{
	Prefab prefab;
};

struct MakeUniquePrefabEdit : EditBase<Edit::MakeUniquePrefab>
{
	uint32_t clientId;
	sf::Symbol prefabName;
	sf::Array<uint32_t> propIds;
};

struct AddPropEdit : EditBase<Edit::AddProp>
{
	Prop prop;
};

struct ClonePropEdit : EditBase<Edit::CloneProp>
{
	uint32_t clientId;
	uint32_t localId;
	Prop prop;
};

struct MovePropEdit : EditBase<Edit::MoveProp>
{
	uint32_t propId;
	PropTransform transform;
};

struct RemovePropEdit : EditBase<Edit::RemoveProp>
{
	uint32_t propId;
};

static const constexpr uint32_t NumServerIdTypes = 100;
static const constexpr uint32_t NumIdTypes = 100;
static const constexpr uint32_t MaxServerIdIndex = 10000000;
static const constexpr uint32_t MaxIdIndex = MaxServerIdIndex * 2;
static_assert(MaxIdIndex < UINT32_MAX / NumIdTypes, "MaxIdIndex overflow");

sf_inline uint32_t makeId(IdType type, uint32_t index)
{
	sf_assert(index < MaxIdIndex);
	return index * NumIdTypes + (uint32_t)type;
}

sf_inline IdType getIdType(uint32_t id)
{
	return (IdType)(id % NumIdTypes);
}

sf_inline uint32_t getIdIndex(uint32_t id)
{
	return id / NumIdTypes;
}

sf_inline bool isIdLocal(uint32_t id)
{
	return getIdIndex(id) >= MaxServerIdIndex;
}

struct KeyName { template <typename T> decltype(T::name)& operator()(T &t) { return t.name; } };
struct KeyId { template <typename T> decltype(T::id)& operator()(T &t) { return t.id; } };

using PrefabMap = sf::ImplicitHashMap<Prefab, KeyName>;
using PropMap = sf::ImplicitHashMap<Prop, KeyId>;
using CharacterMap = sf::ImplicitHashMap<Character, KeyId>;
using CardMap = sf::ImplicitHashMap<Card, KeyId>;
using StatusMap = sf::ImplicitHashMap<Status, KeyId>;

typedef void EventCallbackFn(void *user, Event &event);

sf_inline uint32_t packTile(const sf::Vec2i &tile) {
	return ((uint32_t)tile.x & 0xffff) | ((uint32_t)tile.y << 16u);
}
sf_inline sf::Vec2i unpackTile(uint32_t packed) {
	return sf::Vec2i((int32_t)(int16_t)(packed & 0xffff), (int32_t)packed >> 16u);
}

struct ServerState
{
	ServerState();

	uint32_t localClientId = 0;
	PrefabMap prefabs;
	PropMap props;
	CharacterMap characters;
	CardMap cards;
	StatusMap statuses;
	sf::HashMap<sf::Symbol, int32_t> charactersToSelect;
	sf::HashMap<sf::Symbol, sf::UintSet> prefabProps;

	sf::UintMap tileToEntity;
	sf::UintMap entityToTile;

	uint32_t lastAllocatedIdByType[NumServerIdTypes] = { };
	uint32_t lastLocalAllocatedIdByType[NumServerIdTypes] = { };

	bool isIdValid(uint32_t id);

	void applyEvent(const Event &event);

	void getAsEvents(EventCallbackFn *callback, void *user) const;

	// -- Server only

	uint32_t allocateId(sf::Array<sf::Box<Event>> &events, IdType type, bool local);

	void putStatus(sf::Array<sf::Box<Event>> &events, const StatusInfo &statusInfo);
	void doDamage(sf::Array<sf::Box<Event>> &events, const DamageInfo &damageInfo);
	void castSpell(sf::Array<sf::Box<Event>> &events, const SpellInfo &spellInfo);
	void meleeAttack(sf::Array<sf::Box<Event>> &events, const MeleeInfo &meleeInfo);
	void startCharacterTurn(sf::Array<sf::Box<Event>> &events, uint32_t characterId);

	void preloadPrefab(sf::Array<sf::Box<Event>> &events, const sf::Symbol &name);
	void reloadPrefab(sf::Array<sf::Box<Event>> &events, const Prefab &prefab);

	uint32_t selectCharacterSpawn(sf::Array<sf::Box<Event>> &events, const sf::Symbol &type, uint32_t playerId);

	uint32_t addProp(sf::Array<sf::Box<Event>> &events, const Prop &prop, bool local=false);
	void removeProp(sf::Array<sf::Box<Event>> &events, uint32_t propId);
	uint32_t replaceLocalProp(sf::Array<sf::Box<Event>> &events, const Prop &prop, uint32_t clientId, uint32_t localId);
	uint32_t addCharacter(sf::Array<sf::Box<Event>> &events, const Character &chr, bool local=false);
	uint32_t addCard(sf::Array<sf::Box<Event>> &events, const Card &card, bool local=false);
	void addCharacterToSelect(sf::Array<sf::Box<Event>> &events, const sf::Symbol &type, int32_t count);

	void moveProp(sf::Array<sf::Box<Event>> &events, uint32_t propId, const PropTransform &transform);

	void giveCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId, uint32_t ownerId);
	void selectCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId, uint32_t ownerId, uint32_t slot);

	void applyEdit(sf::Array<sf::Box<Event>> &events, const Edit &edit, sf::Array<sf::Box<Edit>> &undoBuf);

	void garbageCollectIds(sf::Array<uint32_t> &garbageIds) const;
	void garbageCollectPrefabs(sf::Array<sf::Symbol> &garbagePrefabs) const;

	void removeIds(sf::Slice<const uint32_t> ids);
	void removePrefabs(sf::Slice<const sf::Symbol> names);

	// -- Internal

	void addEntityToTile(uint32_t id, const sf::Vec2i &tile);
	void removeEntityFromTile(uint32_t id, const sf::Vec2i &tile);
	void removeEntityFromAllTiles(uint32_t id);
	sf::UintFind getTileEntities(const sf::Vec2i &tile) const;
	sf::UintFind getEntityPackedTiles(uint32_t id) const;
};

}
