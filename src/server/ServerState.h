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

#define sv_reflect(...)

namespace sv {

struct DiceRoll sv_reflect()
{
	uint32_t num = 0;
	uint32_t die = 0;
	uint32_t bias = 0;
	uint32_t check = 0;
};

struct SoundEffect sv_reflect()
{
	sf::Symbol soundName sv_reflect(asset);
	float volume = 1.0f;
	float volumeVariance = 0.0f;
	float pitch = 1.0f;
	float pitchVariance = 0.0f;
	bool loop = false;
};

struct Component
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		DynamicModel,
		TileModel,
		PointLight,
		ParticleSystem,
		Character,
		CharacterModel,
		TapArea,
		Door,
		Chest,
		BlobShadow,
		Card,
		CardAttach,
		CardStatus,
		CardMelee,
		CardKey,
		DamageOnTurnStart,
		CastOnTurnStart,
		CastOnReceiveDamage,
		CastOnDealDamage,
		Projectile,
		ResistDamage,
		IncreaseDamage,
		CardCast,
		CardCastMelee,
		Spell,
		SpellDamage,
		SpellHeal,
		SpellStatus,
		Status,
		StatusChangeTeam,
		CharacterTemplate,
		TileArea,
		Effect,
		Sound,
		RoomConnection,
		Wall,
		GlobalEffectsComponent,

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

struct DynamicModelComponent : ComponentBase<Component::DynamicModel>
{
	sf::Symbol model sv_reflect(asset); //! Model .fbx asset
	sf::Symbol shadowModel sv_reflect(asset); //! Model .fbx used for shadow instead of 'model'
	sf::Symbol material sv_reflect(asset); //! Material used fo the asset
	sf::Vec3 position; //! Offset (in meters) of the model relative to the entity
	sf::Vec3 rotation; //! Rotation (XYZ in degrees) of the model relative to the entity
	float scale = 1.0f; //! Uniform scaling applied to the model
	sf::Vec3 stretch = sf::Vec3(1.0f); //! Non-uniform scaling in entity's local X/Y/Z directions
	uint8_t tintColor[3] = { 255, 255, 255 } sv_reflect(color); //! Modifies the base color of the model's material
	bool castShadows = true; //! Does the model cast shadows?
};

struct TileModelComponent : ComponentBase<Component::TileModel>
{
	sf::Symbol model sv_reflect(asset); //! Model .fbx asset
	sf::Symbol shadowModel sv_reflect(asset); //! Model .fbx used for shadow instead of 'model'
	sf::Symbol material sv_reflect(asset); //! Material used fo the asset
	sf::Symbol giModel sv_reflect(asset); //! Model .fbx used for GI rendering
	sf::Symbol giMaterial sv_reflect(asset); //! Color texture used for GI rendering
	sf::Vec3 position; //! Offset (in meters) of the model relative to the entity
	sf::Vec3 rotation; //! Rotation (XYZ in degrees) of the model relative to the entity
	float scale = 1.0f; //! Uniform scaling applied to the model
	sf::Vec3 stretch = sf::Vec3(1.0f); //! Non-uniform scaling in entity's local X/Y/Z directions
	uint8_t tintColor[3] = { 255, 255, 255 } sv_reflect(color); //! Modifies the base color of the model's material
	bool castShadows = true; //! Does the model cast shadows?
};

struct PointLightComponent : ComponentBase<Component::PointLight>
{
	sf::Vec3 color = sf::Vec3(1.0f) sv_reflect(color); //! Color of the light
	float intensity = 1.0f; //! Brightness of the light
	float radius = 1.0f; //! Maximum eistance in meters that the light can reach
	sf::Vec3 position; //! Offset (in meters) of the light in the entity
	bool hasShadows = true; //! Does this light have shadows
	bool hasBounce = false; //! Does light contribute to the environment lighting
	uint8_t minQuality = 0; //! Minimum quality level needed for this light (0-5)
	float fadeInTime = 0.0f; //! Time to fade in when spawning
	float fadeOutTime = 0.0f; //! Time to fade out when spawning
};

struct BSpline2 sv_reflect()
{
	sf::Array<sf::Vec2> points;
};

struct GradientPoint sv_reflect()
{
	float t;
	sf::Vec3 color;
};

struct Gradient sv_reflect()
{
	sf::Vec3 defaultColor = sf::Vec3(1.0f);
	sf::Array<GradientPoint> points;
};

struct RandomSphere sv_reflect()
{
	float minTheta = 0.0f; //! Minimum angle in degrees of the horizontal arc (0-360)
	float maxTheta = 360.0f; //! Maximum angle in degrees of the horizontal arc (0-360)
	float minPhi = 0.0f; //! Minimum angle in degrees of the vertical arc (0-180)
	float maxPhi = 180.0f; //! Maximums angle in degrees of the vertical arc (0-180)
	float minRadius = 0.0f; //! Minimum distance from the center (in meters)
	float maxRadius = 0.0f; //! Maximum distance from the center (in meters)
	sf::Vec3 scale = sf::Vec3(1.0f); //! Stretches the sphere non-uniformly
};

struct RandomVec3 sv_reflect()
{
	sf::Vec3 offset; //! Base offset/center position
	sf::Vec3 boxExtent; //! Random X/Y/Z box size in meters
	RandomSphere sphere; //! Random sphere
	sf::Vec3 rotation; //! Rotation (X/Y/Z degrees) applied to the final values
};

struct GravityPoint sv_reflect()
{
	sf::Vec3 position; //! Position of the gravity in the local space
	float radius = 0.5f; //! Distance where the force is at maximum
	float strength = 1.0f; //! Strength of the gravitational pull (negative to push away)
};

struct ParticleSystemComponent : ComponentBase<Component::ParticleSystem>
{
	sf::Symbol texture sv_reflect(asset); //! Texture used for the effect
	sf::Vec2i frameCount = sf::Vec2i(1); //! Number of frames in a sprite sheet

	float timeStep = 0.1f; //! How large simulation steps to do, smaller values are heavier but more accurate
	float prewarmTime = 0.0f; //! Time to simulate after spawning the effect

	float updateRadius = 10.0f; //! Size of the "active area" of this particle effect
	float cullPadding = 1.5f; //! How much to "pad" the area for culling, increase if the effect disappears
	bool updateOutOfCamera = false; //! Update even if not visible in the camera
	double renderOrder = 0.0; //! Smaller render order values are rendered first

	float spawnTime = 0.2f; //! Time in seconds between spawning particles
	float spawnTimeVariance = 0.0f; //! Random extra time between spawns
	uint32_t burstAmount = 0; //! How many particles to spawn in the beginning
	uint32_t burstAmountVariance = 0; //! Additional random particle amount to spawn in the beginning
	float emitterOnTime = -1.0f; //! How long to emit particles (negative for infinite, zero for no emit)
	bool localSpace = false; //! Simualte particles in entity-local space
	bool instantDelete = false; //! Delete particles immediately when the entity is removed

	RandomVec3 emitPosition; //! Random position for new particles
	RandomVec3 emitVelocity; //! Random velocity for new particles

	sf::Vec3 emitVelocityAttractorOffset; //! Position to apply extra velocity towards/away from
	float emitVelocityAttractorStrength; //! Extra velocity towards (< 0) or away (> 1) from attractor offset

	sf::Array<GravityPoint> gravityPoints; //! Gravity points to attract/repel particles

	float drag = 0.0f; //! Air resistance slowing particles from moving
	sf::Vec3 gravity; //! Linear force applied to all particles

	float size = 0.5f; //! Base size of particles
	float sizeVariance = 0.0f; //! Random additional size for individual particles

	float lifeTime = 1.0f; //! Time in seconds the particles live
	float lifeTimeVariance = 0.0f; //! Random additional per-particle life time

	BSpline2 scaleSpline; //! Particle scale over particle lifetime
	BSpline2 alphaSpline; //! Particle alpha over particle lifetime
	BSpline2 additiveSpline; //! Additive blending over particle lifetime
	BSpline2 erosionSpline; //! Particle alpha erosion amount
	Gradient gradient; //! Color gradient over particle lifetime

	float frameRate = 0.0f; //! Texture animation speed in frames per second
	bool relativeFrameRate = false; //! Ties the tie frame rate to particle life time (for synchronization)
	bool randomStartFrame = false; //! Start from a random frame in the sprite sheet

	float rotation = 0.0f; //! Base rotation of the particles in degrees
	float rotationVariance = 0.0f; //! Random per-particle extra rotation of the particles in degrees
	float spin = 0.0f; //! Rotation speed for the particles in degrees
	float spinVariance = 0.0f; //! Random per-particle speed for the particles in degrees
};

struct DropCard sv_reflect()
{
	sf::Symbol cardName sv_reflect(prefab); //! Name of the card to drop
	bool alwaysDrop = true; //! Always drop this card
};

struct CharacterComponent : ComponentBase<Component::Character>
{
	sf::Symbol name;
	sf::Symbol description sv_reflect(multiline);
	sf::Symbol statusActiveIcon sv_reflect(asset);
	sf::Symbol statusInactiveIcon sv_reflect(asset);
	uint32_t maxHealth = 20;
	uint32_t baseArmor = 0;
	uint32_t minWeightDice = 1;
	uint32_t maxWeightDice = 100;
	uint32_t meleeSlots = 1;
	uint32_t skillSlots = 0;
	uint32_t spellSlots = 0;
	uint32_t itemSlots = 0;
	uint32_t baseSpeed = 5;
	sf::Vec3 centerOffset = sf::Vec3(0.0f, 0.5f, 0.0f);
	sf::Symbol defeatEffect sv_reflect(prefab);
	SoundEffect damageSound;
	SoundEffect footstepSound;
};

struct AnimationEvent sv_reflect()
{
	sf::Symbol name; //! Name of the event
	float time = 0.0f; //! Time into the animation when the event is triggered
};

struct AnimationInfo sv_reflect()
{
	sf::Array<sf::Symbol> tags; //! Tags for when to play the animation
	sf::Symbol file sv_reflect(asset); //! Animation .fbx asset file
	sf::Array<AnimationEvent> events; //! Events in the animation
	float weight = 1.0f; //! How often to play the animation (for equal tags)
	bool loop = true; //! Should the animation blend the ending with the beginning
	float speed = 1.0f; //! How fast to play the animation
	float speedVariation = 0.0f; //! Random playback speed variation added on top of speed
	float fadeInDuration = 0.3f; //! Time in seconds for the animation to fade in
	float fadeOutDuration = 0.3f; //! Time in seconds for the animation to fade out
	float startTime = 0.0f; //! Offset time to start into the animation
};

struct AttachBone sv_reflect()
{
	sf::Symbol name; //! Name of the attachment point eg. ItemL
	sf::Symbol boneName; //! Name of the .fbx bone eg. bnd_object
	float scale = 1.0f; //! Scale factor for attached objects
};

struct CharacterMaterial sv_reflect()
{
	sf::Symbol name; //! Name of the material in the .fbx file
	sf::Symbol material sv_reflect(asset); //! Material asset name
};

struct CharacterModelComponent : ComponentBase<Component::CharacterModel>
{
	sf::Symbol modelName sv_reflect(asset); //! Character model asset
	sf::Symbol giModel sv_reflect(asset); //! Model used for environment lighting
	sf::Symbol giMaterial sv_reflect(asset); //! Color texture used for environment rendering
	sf::Array<CharacterMaterial> materials; //! Material asset links
	float scale = 1.0f; //! Scale of the character
	sf::Array<AnimationInfo> animations;
	sf::Array<AttachBone> attachBones; //! Bone mapping for attaching objects
};

struct TapAreaComponent : ComponentBase<Component::TapArea>
{
	sf::Vec3 offset; //! Offset from the character
	sf::Vec3 extent = sf::Vec3(1.0f); //! Size of the clickable/tappable box
};

struct DoorComponent : ComponentBase<Component::Door>
{
	sf::Array<sf::Symbol> keyNames; //! Names of keys that fit the door
	SoundEffect openSound; //! Played when the door opens
};

struct ChestDrop sv_reflect()
{
	sf::Symbol cardPrefab sv_reflect(prefab); //! Name of the card to drop
};

struct ChestComponent : ComponentBase<Component::Chest>
{
	sf::Array<ChestDrop> drops;
	sf::Array<sf::Symbol> keyNames; //! Names of keys that fit the chest
	SoundEffect openSound; //! Played when the chest opens
	sf::Symbol openEffect sv_reflect(prefab); //! Effect to spawn when opened
	sf::Vec3 openEffectOffset; //! Offset where openEffect is spawned
};

struct ShadowBlob sv_reflect()
{
	sf::Symbol boneName;
	float radius = 0.0f;
	float alpha = 0.0f;
	float fadeDistance = 0.0f;
	sf::Vec3 offset;
};

struct BlobShadowComponent : ComponentBase<Component::BlobShadow>
{
	sf::Array<ShadowBlob> blobs;
};

struct CardComponent : ComponentBase<Component::Card>
{
	sf::Symbol image sv_reflect(asset);
	sf::Symbol name;
	sf::Symbol description sv_reflect(multiline);
	uint32_t cooldown = 0;
	// TODO: Enum
	bool melee = false;
	bool skill = false;
	bool spell = false;
	bool item = false;

	bool consumable = false; //! Remove the card after use

	// Targetting
	bool targetSelf = false; //! Allow casting the card to self
	bool targetEnemies = true; //! Allow casting the card to enemies
	bool targetFriends = true; //! Allow casting the card to friends
	bool useMeleeRange = false; //! Use the range of the equipped melee card for this
	int32_t targetRadius = 1; //! Movement distance to target
	int32_t targetBoxRadius = 1; //! Distance including diagonals to target
	bool blockedByCharacter = false; //! Targeting blocked by characters
	bool blockedByProp = false; //! Targeting blocked by props
	bool blockedByWall = false; //! Targeting blocked by walls
};

struct CardAttachComponent : ComponentBase<Component::CardAttach>
{
	sf::Symbol prefabName sv_reflect(prefab);
	sf::Symbol boneName;
	float scale = 1.0f;
	sf::Vec3 offset;
	sf::Array<sf::Symbol> animationTags;
};

struct CardStatusComponent : ComponentBase<Component::CardStatus>
{
	sf::Symbol statusName sv_reflect(prefab);
};

struct CardMeleeComponent : ComponentBase<Component::CardMelee>
{
	DiceRoll hitRoll;
	bool directRoll = false;
	SoundEffect hitSound;
};

struct CardKeyComponent : ComponentBase<Component::CardKey>
{
	sf::Array<sf::Symbol> keyNames; //! Types of things to open
	bool consumable = true; //! Is the key removed after use
};

struct ProjectileComponent : ComponentBase<Component::Projectile>
{
	sf::Symbol prefabName sv_reflect(prefab);
	sf::Symbol hitEffect sv_reflect(prefab);
	float flightSpeed = 1.0f;
};

struct DamageOnTurnStartComponent : ComponentBase<Component::DamageOnTurnStart>
{
	DiceRoll damageRoll;
};

struct CastOnTurnStartComponent : ComponentBase<Component::CastOnTurnStart>
{
	sf::Symbol spellName sv_reflect(prefab);
};

struct CastOnReceiveDamageComponent : ComponentBase<Component::CastOnReceiveDamage>
{
	bool onMelee = false;
	bool onSpell = false;
	sf::Symbol spellName sv_reflect(prefab);
};

struct CastOnDealDamageComponent : ComponentBase<Component::CastOnDealDamage>
{
	bool onMelee = false;
	bool onSpell = false;
	sf::Symbol spellName sv_reflect(prefab);
};

struct ResistDamageComponent : ComponentBase<Component::ResistDamage>
{
	// TODO: Enum
	bool onSpell = false;
	bool onMelee = false;
	float resistAmount = 1.0f;
	DiceRoll successRoll;
	sf::Symbol effectName sv_reflect(prefab);
};

struct IncreaseDamageComponent : ComponentBase<Component::IncreaseDamage>
{
	// TODO: Enum
	bool onSpell = false;
	bool onMelee = false;
	float increaseAmount = 1.0f;
	DiceRoll successRoll;
	sf::Symbol effectName sv_reflect(prefab);
};

struct CardCastComponent : ComponentBase<Component::CardCast>
{
	sf::Symbol spellName sv_reflect(prefab);
};

struct CardCastMeleeComponent : ComponentBase<Component::CardCastMelee>
{
	DiceRoll hitCount;
};

struct SpellComponent : ComponentBase<Component::Spell>
{
	sf::Symbol castEffect sv_reflect(prefab);
	sf::Symbol hitEffect sv_reflect(prefab);
	DiceRoll successRoll;
	bool useItemAnimation = false;
	bool useSkillAnimation = false;
};

struct SpellDamageComponent : ComponentBase<Component::SpellDamage>
{
	DiceRoll damageRoll;
};

struct SpellHealComponent : ComponentBase<Component::SpellHeal>
{
	DiceRoll healRoll;
};

struct SpellStatusComponent : ComponentBase<Component::SpellStatus>
{
	sf::Symbol statusName sv_reflect(prefab);
};

struct StatusComponent : ComponentBase<Component::Status>
{
	DiceRoll turnsRoll;
	sf::Symbol startEffect sv_reflect(prefab);
	sf::Symbol activeEffect sv_reflect(prefab);
	sf::Symbol tickEffect sv_reflect(prefab);
	sf::Symbol endEffect sv_reflect(prefab);
	bool stacks = false;
	bool ticksOnTurnEnd = false;
};

struct StatusChangeTeamComponent : ComponentBase<Component::StatusChangeTeam>
{
	bool temp = false;
};

struct StarterCard sv_reflect()
{
	sf::Symbol prefabName sv_reflect(prefab);
	float probability = 1.0f;
};

struct CharacterTemplateComponent : ComponentBase<Component::CharacterTemplate>
{
	sf::Symbol name;
	sf::Symbol description sv_reflect(multiline);
	sf::Symbol characterPrefab sv_reflect(prefab);
	sf::Array<StarterCard> starterCards;
};

struct TileAreaComponent : ComponentBase<Component::TileArea>
{
	sf::Vec2i minCorner sv_reflect(fixed(16)); //! Top-left corner of the tile area (in meters/tiles)
	sf::Vec2i maxCorner sv_reflect(fixed(16)); //! Bottom-right corner of the tile area (in meters/tiles)
};

struct SoundInfo sv_reflect()
{
	sf::Symbol assetName sv_reflect(asset);
};

struct EffectComponent : ComponentBase<Component::Effect>
{
	float lifeTime = 1.0f;
	bool grounded = false;
};

struct SoundComponent : ComponentBase<Component::Sound>
{
	sf::Array<SoundInfo> sounds;
	float volume = 1.0f;
	float volumeVariance = 0.0f;
	float pitch = 1.0f;
	float pitchVariance = 0.0f;
	bool loop = false;
	sf::Vec3 offset;
};

struct RoomConnectionComponent : ComponentBase<Component::RoomConnection>
{
	sf::Vec2i minCorner sv_reflect(fixed(16)); //! Top-left corner of the room connection (in meters/tiles)
	sf::Vec2i maxCorner sv_reflect(fixed(16)); //! Bottom-right corner of the room connection (in meters/tiles)
	sf::Symbol connectionType; //! Name of the connection that can be matched with this
};

struct WallComponent : ComponentBase<Component::Wall>
{
	bool temp = false;
};

struct GlobalEffectsComponent : ComponentBase<Component::GlobalEffectsComponent>
{
	sf::Symbol meleeHitEffect sv_reflect(prefab);
};


struct Prefab sv_reflect()
{
	sf::Symbol name;
	sf::Array<sf::Box<Component>> components;

	Component *findComponentImp(Component::Type type) const;

	template <typename T>
	T *findComponent() const { return (T*)findComponentImp(T::ComponentType); }
};

struct PropTransform sv_reflect()
{
	sf::Vec2i position      sv_reflect(fixed(16)); // 1/2^16 m
	int32_t offsetY = 0     sv_reflect(fixed(16)); // 1/2^16 m
	uint16_t rotation = 0   sv_reflect(fixed(6));  // 1/2^6 deg
	uint16_t scale = 0x100  sv_reflect(fixed(8));  // 1/2^8 x
};

struct Prop sv_reflect()
{
	/* no-reflect */ enum Flag
	/* no-reflect */ {
	/* no-reflect */ Wall = 0x1,
	/* no-reflect */ NoCollision = 0x2,
	/* no-reflect */ Used = 0x4,
	/* no-reflect */ };

	uint32_t id;
	uint32_t flags = 0;
	PropTransform transform;
	sf::Symbol prefabName sv_reflect(prefab);
};

struct Card sv_reflect()
{
	uint32_t id;
	uint32_t ownerId = 0;
	sf::Symbol prefabName sv_reflect(prefab);
	uint32_t cooldownLeft = 0;
};

struct Status sv_reflect()
{
	uint32_t id;
	uint32_t characterId;
	sf::Symbol prefabName sv_reflect(prefab);
	sf::Symbol cardName sv_reflect(prefab);
	uint32_t originalCasterId;
	uint32_t casterId;
	uint32_t turnsLeft;
	bool ticksOnTurnEnd = false;
};

static const uint32_t NumSelectedCards = 10;

struct Character sv_reflect()
{
	uint32_t id;
	int32_t maxHealth = 0;
	int32_t health = 0;
	sf::Symbol prefabName sv_reflect(prefab);
	uint32_t selectedCards[NumSelectedCards] = { };
	sf::Array<uint32_t> cards;
	sf::Array<uint32_t> statuses;
	sf::Array<DropCard> dropCards;
	sf::Vec2i tile;
	uint32_t armor = 0;
	uint32_t playerClientId = 0;
	bool enemy = false;
	bool originalEnemy = false;
};

struct StatusInfo sv_reflect()
{
	uint32_t originalCasterId;
	uint32_t casterId;
	uint32_t targetId;
	sf::Symbol statusName sv_reflect(prefab);
	sf::Symbol cardName sv_reflect(prefab);
};

struct SpellInfo sv_reflect()
{
	uint32_t originalCasterId;
	uint32_t casterId;
	uint32_t targetId;
	sf::Symbol spellName sv_reflect(prefab);
	sf::Symbol cardName sv_reflect(prefab);
	bool manualCast = false;
};

struct MeleeInfo sv_reflect()
{
	uint32_t attackerId;
	uint32_t targetId;
	sf::Symbol cardName sv_reflect(prefab);
};

struct DamageInfo sv_reflect()
{
	bool melee = false;
	bool physical = false;
	bool magic = false;
	bool passive = false;
	bool weaponRoll = false;
	sf::Symbol cardName sv_reflect(prefab);
	uint32_t originalCasterId;
	uint32_t causeId;
	uint32_t targetId;
	DiceRoll damageRoll;
};

struct HealInfo sv_reflect()
{
	sf::Symbol cardName sv_reflect(prefab);
	uint32_t originalCasterId;
	uint32_t causeId;
	uint32_t targetId;
	DiceRoll healRoll;
};

struct RollInfo sv_reflect()
{
	sf::Symbol name;
	DiceRoll roll;
	sf::SmallArray<uint32_t,2> results;
	uint32_t total;
};

struct CastInfo sv_reflect()
{
	SpellInfo spellInfo sv_reflect(prefab);
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
		CardCooldownStart,
		CardCooldownTick,
		StatusAdd,
		StatusExtend,
		StatusTick,
		ChangeTeam,
		StatusRemove,
		ResistDamage,
		IncreaseDamage,
		UseCard,
		CastSpell,
		MeleeAttack,
		Damage,
		Heal,
		LoadPrefab,
		ReloadPrefab,
		MakeUniquePrefab,
		RemoveGarbageIds,
		RemoveGarbagePrefabs,
		AddProp,
		RemoveProp,
		ReplaceLocalProp,
		SetPropCollision,
		DoorOpen,
		ChestOpen,
		AddCharacter,
		RemoveCharacter,
		AddCard,
		RemoveCard,
		MoveProp,
		GiveCard,
		SelectCard,
		UnselectCard,
		AddCharacterToSpawn,
		SelectCharacterToSpawn,
		Move,
		TweakCharacter,
		TurnUpdate,
		VisibleUpdate,
		LoadGlobals,
		SelectCharacter,
		StartBattle,
		EndBattle,

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

struct CardCooldownStartEvent : EventBase<Event::CardCooldownStart>
{
	uint32_t cardId;
	uint32_t cooldown;
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

struct StatusExtendEvent : EventBase<Event::StatusExtend>
{
	uint32_t statusId;
	RollInfo turnsRoll;
};

struct StatusTickEvent : EventBase<Event::StatusTick>
{
	uint32_t statusId;
};

struct StatusRemoveEvent : EventBase<Event::StatusRemove>
{
	uint32_t statusId;
};

struct ChangeTeamEvent : EventBase<Event::ChangeTeam>
{
	sf::Symbol cardName;
	uint32_t characterId;
	uint32_t playerClientId;
	bool enemy;
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

struct IncreaseDamageEvent : EventBase<Event::IncreaseDamage>
{
	sf::Symbol cardName;
	sf::Symbol effectName;
	float increaseAmount;
	uint32_t increaseDamage;
	RollInfo successRoll;
	bool success;
};

struct UseCardEvent : EventBase<Event::UseCard>
{
	uint32_t characterId;
	uint32_t targetId;
	uint32_t cardId;
};

struct CastSpellEvent : EventBase<Event::CastSpell>
{
	SpellInfo spellInfo;
	RollInfo successRoll;
	bool useItemAnimation;
	bool useSkillAnimation;
};

struct MeleeAttackEvent : EventBase<Event::MeleeAttack>
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
	int32_t damageIncrease = 0;
	int32_t damageDecrease = 0;
};

struct HealEvent : EventBase<Event::Heal>
{
	HealInfo healInfo;
	RollInfo healRoll;
	uint32_t finalHeal = 0;
	uint32_t unclampedFinalHeal = 0;
	int32_t healIncrease = 0;
	int32_t healDecrease = 0;
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

struct SetPropCollisionEvent : EventBase<Event::SetPropCollision>
{
	uint32_t propId;
	bool collisionEnabled;
};

struct DoorOpenEvent : EventBase<Event::DoorOpen>
{
	uint32_t characterId;
	uint32_t propId;
};

struct ChestOpenEvent : EventBase<Event::ChestOpen>
{
	uint32_t characterId;
	uint32_t propId;
};

struct AddCharacterEvent : EventBase<Event::AddCharacter>
{
	Character character;
};

struct RemoveCharacterEvent : EventBase<Event::RemoveCharacter>
{
	uint32_t characterId;
};

struct AddCardEvent : EventBase<Event::AddCard>
{
	Card card;
};

struct RemoveCardEvent : EventBase<Event::RemoveCard>
{
	uint32_t cardId;
	uint32_t prevOwnerId;
};

struct MovePropEvent : EventBase<Event::MoveProp>
{
	uint32_t propId;
	PropTransform transform;
};

struct GiveCardInfo sv_reflect()
{
	bool fromWorld = false;
	sf::Vec2i worldTile;
};

struct GiveCardEvent : EventBase<Event::GiveCard>
{
	uint32_t cardId;
	uint32_t previousOwnerId;
	uint32_t ownerId;
	GiveCardInfo info;
};

struct SelectCardEvent : EventBase<Event::SelectCard>
{
	uint32_t ownerId;
	uint32_t cardId;
	uint32_t slot;
};

struct UnselectCardEvent : EventBase<Event::UnselectCard>
{
	uint32_t ownerId;
	uint32_t prevCardId;
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

struct Waypoint sv_reflect()
{
	sf::Vec2i position;
};

struct MoveEvent : EventBase<Event::Move>
{
	uint32_t characterId;
	sf::Vec2i position;
	sf::Array<Waypoint> waypoints;
	bool instant = false;
};

struct TweakCharacterEvent : EventBase<Event::TweakCharacter>
{
	Character character;
};

struct TurnInfo sv_reflect()
{
	bool startTurn = false;
	uint32_t characterId = 0;
	uint32_t movementLeft = 0;
};

struct TurnUpdateEvent : EventBase<Event::TurnUpdate>
{
	TurnInfo turnInfo;
	bool immediate;
};

struct VisibleTile sv_reflect()
{
	uint32_t packedTile;
	uint32_t amount;
};

struct VisibleUpdateEvent : EventBase<Event::VisibleUpdate>
{
	sf::Array<VisibleTile> visibleTiles;
};

struct GlobalPrefabs sv_reflect()
{
	sf::Symbol effects;
};

struct LoadGlobalsEvent : EventBase<Event::LoadGlobals>
{
	GlobalPrefabs globalPrefabs;
};

struct SelectCharacterEvent : EventBase<Event::SelectCharacter>
{
	uint32_t characterId;
	uint32_t clientId;
};

struct StartBattleEvent : EventBase<Event::StartBattle>
{
	uint32_t characterId;
};

struct EndBattleEvent : EventBase<Event::EndBattle>
{
	uint32_t temp;
};


enum class IdType {
	Null,
	Prop,
	Character,
	Card,
	Status,
	RoomConnection,
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
		AddCharacter,
		RemoveCharacter,
		MoveCharacter,
		TweakCharacter,
		AddCard,
		ReplaceCard,
		RemoveCard,

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

struct AddCharacterEdit : EditBase<Edit::AddCharacter>
{
	Character character;
};

struct RemoveCharacterEdit : EditBase<Edit::RemoveCharacter>
{
	uint32_t characterId;
};

struct MoveCharacterEdit : EditBase<Edit::MoveCharacter>
{
	uint32_t characterId;
	sf::Vec2i position;
};

struct TweakCharacterEdit : EditBase<Edit::TweakCharacter>
{
	Character character;
};

struct AddCardEdit : EditBase<Edit::AddCard>
{
	uint32_t characterId;
	sf::Symbol cardName;
	uint32_t slotIndex = ~0u;
};

struct RemoveCardEdit : EditBase<Edit::RemoveCard>
{
	uint32_t cardId;
};

struct Action
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		Move,
		SelectCard,
		GiveCard,
		EndTurn,
		SelectCharacter,
		UseCard,
		OpenDoor,
		OpenChest,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Action() { }
	Action(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::ActionType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::ActionType ? (T*)this : nullptr; }
};

template <Action::Type SelfType>
struct ActionBase : Action
{
	static constexpr Type ActionType = SelfType;
	ActionBase() : Action(SelfType) { }
};

struct MoveAction : ActionBase<Action::Move>
{
	uint32_t characterId;
	sf::Vec2i tile;
	sf::Array<sf::Vec2i> waypoints;
};

struct SelectCardAction : ActionBase<Action::SelectCard>
{
	uint32_t ownerId;
	uint32_t cardId;
	uint32_t slot;
};

struct GiveCardAction : ActionBase<Action::GiveCard>
{
	uint32_t ownerId;
	uint32_t cardId;
};

struct EndTurnAction : ActionBase<Action::EndTurn>
{
	uint32_t characterId;
	bool onlyNonBattle = false;
};

struct SelectCharacterAction : ActionBase<Action::SelectCharacter>
{
	uint32_t clientId;
	uint32_t characterId;
};

struct UseCardAction : ActionBase<Action::UseCard>
{
	uint32_t characterId;
	uint32_t targetId;
	uint32_t cardId;
};

struct OpenDoorAction : ActionBase<Action::OpenDoor>
{
	uint32_t characterId;
	uint32_t doorId;
	uint32_t cardId;
};

struct OpenChestAction : ActionBase<Action::OpenChest>
{
	uint32_t characterId;
	uint32_t chestId;
	uint32_t cardId;
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

struct RoomTiles
{
	sf::HashSet<sf::Vec2i> interior;
	sf::HashSet<sf::Vec2i> border;

	void clear();
};

struct ServerState
{
	ServerState();

	// Not serialized!
	sf::Array<sf::StringBuf> errors;
	uint32_t localClientId = 0;

	PrefabMap prefabs;
	PropMap props;
	CharacterMap characters;
	CardMap cards;
	StatusMap statuses;
	sf::HashMap<sf::Symbol, int32_t> charactersToSelect;
	sf::HashMap<sf::Symbol, sf::UintSet> prefabProps;
	GlobalPrefabs globalPrefabs;

	sf::UintMap tileToEntity;
	sf::UintMap entityToTile;

	sf::UintMap visibleTiles;

	uint32_t lastAllocatedIdByType[NumServerIdTypes] = { };
	uint32_t lastLocalAllocatedIdByType[NumServerIdTypes] = { };

	sf::Array<uint32_t> turnOrder;
	TurnInfo turnInfo;
	uint32_t turnCharacterIndex = 0;

	bool inBattle = false;

	bool isIdValid(uint32_t id);

	void applyEvent(const Event &event);

	void getAsEvents(EventCallbackFn *callback, void *user) const;

	bool canTarget(uint32_t selfId, uint32_t targetId, const sf::Symbol &cardName, const sf::Vec2i &selfTile) const;
	bool canTarget(uint32_t selfId, uint32_t targetId, const sf::Symbol &cardName) const;

	// -- Server only

	uint32_t allocateId(sf::Array<sf::Box<Event>> &events, IdType type, bool local);

	void putStatus(sf::Array<sf::Box<Event>> &events, const StatusInfo &statusInfo);
	void doDamage(sf::Array<sf::Box<Event>> &events, const DamageInfo &damageInfo);
	void doHeal(sf::Array<sf::Box<Event>> &events, const HealInfo &healInfo);
	void castSpell(sf::Array<sf::Box<Event>> &events, const SpellInfo &spellInfo);
	void meleeAttack(sf::Array<sf::Box<Event>> &events, const MeleeInfo &meleeInfo);
	void processDeadCharacters(sf::Array<sf::Box<Event>> &events, bool startNextTurnIfNecessary);

	void updateCharacterVisibility(sf::Array<sf::Box<Event>> &events, uint32_t characterId);

	uint32_t getNextTurnCharacter() const;
	void startNextCharacterTurn(sf::Array<sf::Box<Event>> &events, bool immediate);

	void preloadPrefab(sf::Array<sf::Box<Event>> &events, const sf::Symbol &name);
	void reloadPrefab(sf::Array<sf::Box<Event>> &events, const Prefab &prefab);

	uint32_t selectCharacterSpawn(sf::Array<sf::Box<Event>> &events, const sf::Symbol &type, uint32_t playerId);

	uint32_t addProp(sf::Array<sf::Box<Event>> &events, const Prop &prop, bool local=false);
	void removeProp(sf::Array<sf::Box<Event>> &events, uint32_t propId);
	uint32_t replaceLocalProp(sf::Array<sf::Box<Event>> &events, const Prop &prop, uint32_t clientId, uint32_t localId);
	uint32_t addCharacter(sf::Array<sf::Box<Event>> &events, const Character &chr, bool local=false);
	uint32_t addCard(sf::Array<sf::Box<Event>> &events, const Card &card, bool local=false);
	void removeCharacter(sf::Array<sf::Box<Event>> &events, uint32_t characterId, bool startNextTurnIfNecessary);
	void removeCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId);
	void addCharacterToSelect(sf::Array<sf::Box<Event>> &events, const sf::Symbol &type, int32_t count);

	void moveProp(sf::Array<sf::Box<Event>> &events, uint32_t propId, const PropTransform &transform);

	void giveCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId, uint32_t ownerId, const GiveCardInfo &info);
	void giveCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId, uint32_t ownerId);
	void selectCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId, uint32_t ownerId, uint32_t slot);

	void applyEdit(sf::Array<sf::Box<Event>> &events, const Edit &edit, sf::Array<sf::Box<Edit>> &undoBuf);
	bool requestAction(sf::Array<sf::Box<Event>> &events, const Action &action);

	void startBattle(sf::Array<sf::Box<Event>> &events, uint32_t characterId);
	void endBattle(sf::Array<sf::Box<Event>> &events);

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

	void loadCanonicalPrefabs(sf::Array<sf::Box<sv::Event>> &events);
	void loadGlobals(sf::Array<sf::Box<Event>> &events);
};

struct SavedMap
{
	sf::Box<ServerState> state;
};

}
