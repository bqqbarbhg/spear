#include "ServerState.h"

#include "sf/HashSet.h"
#include "sf/Mutex.h"

#include "ext/json_input.h"
#include "sp/Json.h"
#include "sf/File.h"

#include "sf/Reflection.h"

#include <stdarg.h>

namespace sv {

Component *Prefab::findComponentImp(Component::Type type) const
{
	for (Component *component : components) {
		if (component->type == type) return component;
	}
	return nullptr;
}

sf_inline Prefab *findPrefabExisting(ServerState &state, const sf::Symbol &name)
{
	Prefab *prefab = state.prefabs.find(name);
	if (!prefab) {
		sf::debugPrintLine("Could not find prefab: %s", name.data);
	}
	return prefab;
}

sf_inline Character *findCharacter(ServerState &state, uint32_t id)
{
	Character *chr = state.characters.find(id);
	if (!chr) {
		sf::debugPrintLine("Could not find character: %u", id);
	}
	return chr;
}

sf_inline Card *findCard(ServerState &state, uint32_t id)
{
	Card *card = state.cards.find(id);
	if (!card) {
		sf::debugPrintLine("Could not find card: %u", id);
	}
	return card;
}

sf_inline Status *findStatus(ServerState &state, uint32_t id)
{
	Status *status = state.statuses.find(id);
	if (!status) {
		sf::debugPrintLine("Could not find status: %u", id);
	}
	return status;
}

template <typename T>
sf_inline T *findComponent(const Prefab &prefab)
{
	T *t = (T*)prefab.findComponentImp(T::ComponentType);
	if (!t) {
		sf::debugPrintLine("Could not find component %u from %s", (uint32_t)T::ComponentType, prefab.name);
	}
	return t;
}

sf_inline bool check(bool cond, const char *msg)
{
	if (!cond) { sf::debugPrintLine("Error: %s", msg); }
	return cond;
}

#define sv_check(cond) check((cond), #cond)

static uint32_t rollDie(uint32_t max)
{
	// TODO: Proper random
	return rand() % (max - 1) + 1;
}

static RollInfo rollDice(const DiceRoll &roll, sf::Symbol name)
{
	RollInfo info;
	info.roll = roll;
	uint32_t total = roll.bias;
	for (uint32_t i = 0; i < roll.num; i++) {
		uint32_t value = rollDie(roll.die);
		total += value;
		info.results.push(value);
	}
	info.total = total;
	return info;
}

sf_inline RollInfo rollDice(const DiceRoll &roll, const char *name)
{
	return rollDice(roll, sf::Symbol(name));
}

ServerState::ServerState()
{
	// TODO: Figure something proper out for this
	prefabs.reserve(1024);
	props.reserve(1024);
	characters.reserve(1024);
	cards.reserve(1024);
	statuses.reserve(1024);
	charactersToSelect.reserve(1024);
}

uint32_t ServerState::allocateId(IdType type)
{
	uint32_t &nextId = nextIdByType[(uint32_t)type];
	for (;;) {
		nextId++;
		if (nextId >= MaxIdIndex) nextId = 1;

		uint32_t id = makeId(type, nextId);

		bool exists = false;
		switch (type) {
		case IdType::Prop: exists = !!props.find(id); break;
		case IdType::Character: exists = !!characters.find(id); break;
		case IdType::Card: exists = !!cards.find(id); break;
		case IdType::Status: exists = !!statuses.find(id); break;
		default: sf_failf("Unhandled ID type %u", (uint32_t)type);
		}

		if (!exists) return id;
	}
}

void ServerState::applyEvent(const Event &event)
{
	// TODO: What to do about reallocations?
	if (auto *e = event.as<CardCooldownTickEvent>()) {
		if (Card *card = findCard(*this, e->cardId)) {
			if (sv_check(card->cooldownLeft > 0)) {
				card->cooldownLeft--;
			}
		}
	} else if (auto *e = event.as<StatusAddEvent>()) {
		auto res = statuses.insertOrAssign(e->status);
		sv_check(res.inserted);
		if (Character *chr = findCharacter(*this, e->status.characterId)) {
			chr->statuses.push(e->status.id);
		}
	} else if (auto *e = event.as<StatusTickEvent>()) {
		if (Status *status = findStatus(*this, e->statusId)) {
			if (sv_check(status->turnsLeft > 0)) {
				status->turnsLeft--;
			}
		}
	} else if (auto *e = event.as<StatusRemoveEvent>()) {
		if (Status *status = findStatus(*this, e->statusId)) {
			status->deleted = true;
			if (Character *chr = findCharacter(*this, status->characterId)) {
				sf::findRemoveSwap(chr->statuses, e->statusId);
			}
		}
	} else if (auto *e = event.as<DamageEvent>()) {
		if (Character *chr = findCharacter(*this, e->damageInfo.targetId)) {
			chr->health -= (int32_t)e->finalDamage;
		}
	} else if (auto *e = event.as<LoadPrefabEvent>()) {
		auto res = prefabs.insertOrAssign(e->prefab);
		sv_check(res.inserted);
	} else if (auto *e = event.as<RemoveGarbageIdsEvent>()) {
		removeIds(e->ids.slice());
	} else if (auto *e = event.as<RemoveGarbagePrefabsEvent>()) {
		removePrefabs(e->names.slice());
	} else if (auto *e = event.as<AddPropEvent>()) {
		auto res = props.insertOrAssign(e->prop);
		sv_check(res.inserted);
	} else if (auto *e = event.as<AddCharacterEvent>()) {
		auto res = characters.insertOrAssign(e->character);
		sv_check(res.inserted);
	} else if (auto *e = event.as<AddCardEvent>()) {
		auto res = cards.insertOrAssign(e->card);
		sv_check(res.inserted);
	} else if (auto *e = event.as<GiveCardEvent>()) {
		if (Card *card = findCard(*this, e->cardId)) {
			sv_check(card->ownerId == e->previousOwnerId);

			if (card->ownerId) {
				if (Character *chr = findCharacter(*this, card->ownerId)) {
					for (uint32_t &idRef : chr->selectedCards) {
						if (idRef == card->id) {
							idRef = 0; 
						}
					}
					sf::findRemoveSwap(chr->cards, card->id);
				}
			}

			if (e->ownerId) {
				if (Character *chr = findCharacter(*this, card->ownerId)) {
					chr->cards.push(e->cardId);
				}
			}

			card->ownerId = e->ownerId;
		}
	} else if (auto *e = event.as<AddCharacterToSpawn>()) {
		charactersToSelect[e->selectPrefab] += e->count;
	} else if (auto *e = event.as<SelectCharacterToSpawnEvent>()) {
		charactersToSelect[e->selectPrefab]--;
	}
}

sf_inline void pushEvent(ServerState &state, sf::Array<sf::Box<Event>> &events, const sf::Box<Event> &event)
{
	state.applyEvent(*event);
	events.push(event);
}

static sf::StaticRecursiveMutex g_configMutex;

template <typename T>
static sf::Box<T> loadConfig(sf::String name)
{
	sf::Symbol path = sf::Symbol(name);

	sf::RecursiveMutexGuard mg(g_configMutex);

	static sf::HashMap<sf::Symbol, sf::Box<T>> cache;
	sf::Box<T> &entry = cache[path];
	if (entry) return entry;

	jsi_args args = { };
	args.dialect.allow_bare_keys = true;
	args.dialect.allow_comments = true;
	args.dialect.allow_control_in_string = true;
	args.dialect.allow_missing_comma = true;
	args.dialect.allow_trailing_comma = true;
	jsi_value *value = jsi_parse_file(path.data, &args);
	if (!value) {
		sf::debugPrint("Failed to parse %s:%u:%u: %s",
			path.data, args.error.line, args.error.column, args.error.description);
		return { };
	}

	if (!sp::readJson(value, entry)) return { };

	entry->name = path;

	return entry;
}

static void walkPrefabs(ServerState &state, sf::Array<sf::Box<Event>> *events, sf::HashSet<sf::Symbol> *marks, const sf::Symbol &name)
{
	if (!name) return;
	if (marks) {
		if (!marks->insert(name).inserted) return;
	}
	Prefab *prefab = state.prefabs.find(name);

	if (events) {
		if (prefab) return;

		sf::Box<Prefab> box = loadConfig<Prefab>(name);
		if (!box) return;
		
		{
			auto e = sf::box<LoadPrefabEvent>();
			e->prefab = *box;
			pushEvent(state, *events, e);
		}
		prefab = box;
	}

	for (Component *component : prefab->components) {
		if (auto *c = component->as<CardAttachComponent>()) {
			walkPrefabs(state, events, marks, c->prefabName);
		} else if (auto *c = component->as<CardProjectileComponent>()) {
			walkPrefabs(state, events, marks, c->prefabName);
		} else if (auto *c = component->as<CastOnReceiveDamageComponent>()) {
			walkPrefabs(state, events, marks, c->spellName);
		} else if (auto *c = component->as<CastOnDealDamageComponent>()) {
			walkPrefabs(state, events, marks, c->spellName);
		} else if (auto *c = component->as<CardCastComponent>()) {
			walkPrefabs(state, events, marks, c->spellName);
		} else if (auto *c = component->as<SpellStatusComponent>()) {
			walkPrefabs(state, events, marks, c->statusName);
		} else if (auto *c = component->as<CharacterTemplateComponent>()) {
			walkPrefabs(state, events, marks, c->characterPrefab);
			for (const sf::Symbol &cardPrefab : c->starterCardPrefabs) {
				walkPrefabs(state, events, marks, cardPrefab);
			}
		}
	}
}

sf_inline Prefab *loadPrefab(ServerState &state, sf::Array<sf::Box<Event>> &events, const sf::Symbol &name)
{
	if (Prefab *prefab = state.prefabs.find(name)) {
		return prefab;
	}

	walkPrefabs(state, &events, nullptr, name);

	return state.prefabs.find(name);
}

void ServerState::putStatus(sf::Array<sf::Box<Event>> &events, const StatusInfo &statusInfo)
{
	Prefab *statusPrefab = loadPrefab(*this, events, statusInfo.statusName);
	if (!statusPrefab) return;

	StatusComponent *statusComp = findComponent<StatusComponent>(*statusPrefab);
	if (!statusComp) return;

	uint32_t id = allocateId(IdType::Status);

	{
		auto e = sf::box<StatusAddEvent>();
		e->turnsRoll = rollDice(statusComp->turnsRoll, "turns");
		e->status.id = id;
		e->status.prefabName = statusInfo.statusName;
		e->status.cardName = statusInfo.cardName;
		e->status.originalCasterId = statusInfo.originalCasterId;
		e->status.casterId = statusInfo.casterId;
		e->status.characterId = statusInfo.targetId;
		e->status.turnsLeft = e->turnsRoll.total;
		pushEvent(*this, events, e);
	}
}

void ServerState::doDamage(sf::Array<sf::Box<Event>> &events, const DamageInfo &damageInfo)
{
	Character *causeChr = findCharacter(*this, damageInfo.causeId);
	Character *targetChr = findCharacter(*this, damageInfo.targetId);
	if (!causeChr || !targetChr) return;

	auto e = sf::box<DamageEvent>();
	e->damageInfo = damageInfo;
	e->damageRoll = rollDice(damageInfo.damageRoll, "damage");
	e->meleeArmor = targetChr->armor;

	if (damageInfo.melee) {
		uint32_t total = 0;
		for (uint32_t val : e->damageRoll.results) {
			// TODO: > ?
			uint32_t damage = e->damageRoll.total;
			if (val == e->damageRoll.roll.die) {
				// Crit doubles melee damage
				damage *= 2;
			}
			if (val < e->meleeArmor) {
				// Armor halves melee damage
				damage /= 2;
			}
			total += damage;
		}
		e->finalDamage = total;
	} else {
		e->finalDamage = e->damageRoll.total;
	}

	pushEvent(*this, events, e);

	for (uint32_t statusId : causeChr->statuses) {
		Status *status = findStatus(*this, statusId);
		if (!status) continue;
		Prefab *statusPrefab = loadPrefab(*this, events, status->prefabName);
		if (!statusPrefab) continue;

		for (Component *statusComponent : statusPrefab->components) {
			if (auto *c = statusComponent->as<CastOnDealDamageComponent>()) {
				if ((c->onSpell && damageInfo.magic) || (c->onMelee && damageInfo.melee)) {
					SpellInfo spell = { };
					spell.spellName = c->spellName;
					spell.cardName = status->cardName;
					spell.originalCasterId = status->originalCasterId;
					spell.casterId = damageInfo.causeId;
					spell.targetId = damageInfo.targetId;
					castSpell(events, spell);
				}
			}
		}
	}

	for (uint32_t statusId : targetChr->statuses) {
		Status *status = findStatus(*this, statusId);
		if (!status) continue;
		Prefab *statusPrefab = loadPrefab(*this, events, status->prefabName);
		if (!statusPrefab) continue;

		for (Component *statusComponent : statusPrefab->components) {
			if (auto *c = statusComponent->as<CastOnReceiveDamageComponent>()) {
				if ((c->onSpell && damageInfo.magic) || (c->onMelee && damageInfo.melee)) {
					SpellInfo spell = { };
					spell.spellName = c->spellName;
					spell.cardName = status->cardName;
					spell.originalCasterId = status->originalCasterId;
					spell.casterId = damageInfo.targetId;
					spell.targetId = damageInfo.causeId;
					castSpell(events, spell);
				}
			}
		}
	}
}

void ServerState::castSpell(sf::Array<sf::Box<Event>> &events, const SpellInfo &spellInfo)
{
	Prefab *spellPrefab = loadPrefab(*this, events, spellInfo.spellName);
	if (!spellPrefab) return;

	SpellComponent *spellComp = findComponent<SpellComponent>(*spellPrefab);
	if (!spellComp) return;

	RollInfo successRoll = rollDice(spellComp->successRoll, "success");

	{
		auto e = sf::box<CastSpellEvent>();
		e->spellInfo = spellInfo;
		e->successRoll = successRoll;
		pushEvent(*this, events, e);
	}

	if (successRoll.total < successRoll.roll.check) return;

	for (Component *component : spellPrefab->components) {

		if (auto *c = component->as<SpellDamageComponent>()) {
			RollInfo damageRoll = rollDice(c->damageRoll, "damage");
			DamageInfo damage = { };
			damage.magic = true;
			damage.spellName = spellInfo.spellName;
			damage.originalCasterId = spellInfo.originalCasterId;
			damage.causeId = spellInfo.casterId;
			damage.targetId = spellInfo.targetId;
			damage.damageRoll = c->damageRoll;
			doDamage(events, damage);
		} else if (auto *c = component->as<SpellStatusComponent>()) {
			StatusInfo status = { };
			status.statusName = c->statusName;
			status.cardName = spellInfo.cardName;
			status.originalCasterId = spellInfo.originalCasterId;
			status.casterId = spellInfo.casterId;
			status.targetId = spellInfo.targetId;
			putStatus(events, status);
		}
	}
}

void ServerState::meleeAttack(sf::Array<sf::Box<Event>> &events, const MeleeInfo &meleeInfo)
{
	Character *attackerChr = findCharacter(*this, meleeInfo.attackerId);
	if (!attackerChr) return;

	if (!attackerChr->selectedCards[0]) return;
	Card *card = findCard(*this, attackerChr->selectedCards[0]);
	if (!card) return;

	Prefab *cardPrefab = loadPrefab(*this, events, card->prefabName);
	if (!cardPrefab) return;

	CardMeleeComponent *meleeComponent = findComponent<CardMeleeComponent>(*cardPrefab);
	if (!meleeComponent) return;

	DamageInfo damage = { };
	damage.melee = true;
	damage.physical = true;
	damage.originalCasterId = meleeInfo.attackerId;
	damage.causeId = meleeInfo.attackerId;
	damage.targetId = meleeInfo.targetId;
	damage.damageRoll = meleeComponent->hitRoll;
	doDamage(events, damage);
}

void ServerState::startCharacterTurn(sf::Array<sf::Box<Event>> &events, uint32_t characterId)
{
	Character *chr = findCharacter(*this, characterId);
	if (!chr) return;

	for (uint32_t cardId : chr->cards) {
		Card *card = findCard(*this, cardId);
		if (!card) continue;

		if (card->cooldownLeft > 0) {
			auto e = sf::box<CardCooldownTickEvent>();
			e->cardId = cardId;
			pushEvent(*this, events, std::move(e));
		}
	}

	sf::SmallArray<uint32_t, 64> statusIds;
	statusIds.push(chr->statuses.slice());
	for (uint32_t id : statusIds) {
		Status *status = statuses.find(id);
		if (!status) continue;

		if (Prefab *statusPrefab = loadPrefab(*this, events, status->prefabName)) {
			for (Component *component : statusPrefab->components) {
				if (auto *c = component->as<CastOnTurnStartComponent>()) {
					SpellInfo spell = { };
					spell.originalCasterId = status->originalCasterId;
					spell.casterId = status->casterId;
					spell.targetId = status->characterId;
					spell.cardName = status->cardName;
					spell.spellName = c->spellName;
					castSpell(events, spell);
				}
			}
		}

		if (status->turnsLeft > 0) {
			auto e = sf::box<StatusTickEvent>();
			e->statusId = status->id;
			pushEvent(*this, events, std::move(e));
		} else {
			auto e = sf::box<StatusRemoveEvent>();
			e->statusId = status->id;
			pushEvent(*this, events, std::move(e));
		}
	}
}

uint32_t ServerState::selectCharacterSpawn(sf::Array<sf::Box<Event>> &events, const sf::Symbol &type, uint32_t playerId)
{
	int32_t *left = charactersToSelect.findValue(type);
	if (!left || *left == 0) return 0;

	Prefab *selectPrefab = loadPrefab(*this, events, type);
	if (!selectPrefab) return 0;

	CharacterTemplateComponent *templateComp = findComponent<CharacterTemplateComponent>(*selectPrefab);
	if (!templateComp) return 0;

	{
		auto e = sf::box<SelectCharacterToSpawnEvent>();
		e->selectPrefab = type;
		e->playerId = playerId;
		pushEvent(*this, events, e);
	}

	Character chrProto = { };
	chrProto.prefabName = templateComp->characterPrefab;
	uint32_t chrId = addCharacter(events, chrProto);
	if (!chrId) return 0 ;

	for (const sf::Symbol &cardPrefab : templateComp->starterCardPrefabs) {
		Card cardProto = { };
		cardProto.prefabName = cardPrefab;
		uint32_t cardId = addCard(events, cardProto);
		if (!cardId) return 0;

		giveCard(events, cardId, chrId);
	}

	return chrId;
}

uint32_t ServerState::addProp(sf::Array<sf::Box<Event>> &events, const Prop &prop)
{
	Prefab *prefab = loadPrefab(*this, events, prop.prefabName);
	if (!prefab) return 0;

	uint32_t id = allocateId(IdType::Prop);

	{
		auto e = sf::box<AddPropEvent>();
		e->prop = prop;
		e->prop.id = id;
		pushEvent(*this, events, e);
	}

	return id;
}

uint32_t ServerState::addCharacter(sf::Array<sf::Box<Event>> &events, const Character &chr)
{
	Prefab *prefab = loadPrefab(*this, events, chr.prefabName);
	if (!prefab) return 0;

	CharacterComponent *chrComp = findComponent<CharacterComponent>(*prefab);
	if (!chrComp) return 0;

	uint32_t id = allocateId(IdType::Character);

	{
		auto e = sf::box<AddCharacterEvent>();
		e->character = chr;
		e->character.health = e->character.maxHealth = chrComp->maxHealth;
		e->character.armor = chrComp->baseArmor;
		e->character.id = id;
		pushEvent(*this, events, e);
	}

	return id;
}

uint32_t ServerState::addCard(sf::Array<sf::Box<Event>> &events, const Card &card)
{
	Prefab *prefab = loadPrefab(*this, events, card.prefabName);
	if (!prefab) return 0;

	CharacterComponent *chrComp = findComponent<CharacterComponent>(*prefab);
	if (!chrComp) return 0;

	uint32_t id = allocateId(IdType::Card);

	{
		auto e = sf::box<AddCardEvent>();
		e->card = card;
		e->card.ownerId = 0;
		e->card.id = id;
		pushEvent(*this, events, e);
	}

	return id;
}

void ServerState::addCharacterToSelect(sf::Array<sf::Box<Event>> &events, const sf::Symbol &type, int32_t count)
{
	Prefab *selectPrefab = loadPrefab(*this, events, type);
	if (!selectPrefab) return;

	CharacterTemplateComponent *templateComp = findComponent<CharacterTemplateComponent>(*selectPrefab);
	if (!templateComp) return;

	{
		auto e = sf::box<AddCharacterToSpawn>();
		e->selectPrefab = type;
		e->count = count;
		pushEvent(*this, events, e);
	}
}

void ServerState::giveCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId, uint32_t ownerId)
{
	Card *card = findCard(*this, cardId);
	if (!card) return;

	if (card->ownerId == ownerId) return;

	{
		auto e = sf::box<GiveCardEvent>();
		e->cardId = cardId;
		e->previousOwnerId = ownerId;
		e->ownerId = ownerId;
		pushEvent(*this, events, e);
	}

	if (ownerId) {
		Character *chr = findCharacter(*this, ownerId);
		if (!chr) return;

		Prefab *chrPrefab = loadPrefab(*this, events, chr->prefabName);
		if (!chrPrefab) return;

		CharacterComponent *chrComp = findComponent<CharacterComponent>(*chrPrefab);
		if (!chrComp) return;

		Prefab *cardPrefab = loadPrefab(*this, events, card->prefabName);
		if (!cardPrefab) return;

		CardComponent *cardComp = findComponent<CardComponent>(*cardPrefab);
		if (!cardComp) return;

		uint32_t lastMeleeSlot = 1;
		uint32_t lastSkillSlot = lastMeleeSlot + chrComp->skillSlots;
		uint32_t lastSpellSlot = lastSkillSlot + chrComp->spellSlots;
		uint32_t lastItemSlot = lastSpellSlot + chrComp->itemSlots;

		for (uint32_t i = 0; i < NumSelectedCards; i++) {
			if (chr->selectedCards[i] != 0) continue;

			bool select = false;
			if (i < lastMeleeSlot) {
				select = cardComp->melee;
			} else if (i < lastSkillSlot) {
				select = cardComp->skill;
			} else if (i < lastSpellSlot) {
				select = cardComp->spell;
			} else if (i < lastItemSlot) {
				select = cardComp->item;
			}
			if (select) {
				selectCard(events, cardId, ownerId, i);
			}
		}
	}
}

void ServerState::selectCard(sf::Array<sf::Box<Event>> &events, uint32_t cardId, uint32_t ownerId, uint32_t slot)
{
	{
		auto e = sf::box<SelectCardEvent>();
		e->cardId = cardId;
		e->ownerId = ownerId;
		e->slot = slot;

	}
}

static void markId(const ServerState &state, sf::HashSet<uint32_t> &marks, uint32_t id)
{
	if (marks.insert(id).inserted) return;

	switch (getIdType(id)) {

	case IdType::Prop:
		break;

	case IdType::Character:
		if (const Character *chr = state.characters.find(id)) {
			for (uint32_t id : chr->cards) {
				markId(state, marks, id);
			}
			for (uint32_t id : chr->statuses) {
				markId(state, marks, id);
			}
		}
		break;

	case IdType::Card:
		break;

	case IdType::Status:
		break;

	default:
		sf_failf("Invalid ID type: %u", (uint32_t)getIdType(id));
		break;

	}
}

void ServerState::garbageCollectIds(sf::Array<uint32_t> &garbageIds) const
{
	sf::HashSet<uint32_t> marks;

	for (const Prop &prop : props) {
		if (prop.deleted) continue;
		markId(*this, marks, prop.id);
	}

	for (const Character &chr : characters) {
		if (chr.deleted) continue;
		markId(*this, marks, chr.id);
	}

	for (const Card &card : cards) {
		if (card.deleted) continue;
		markId(*this, marks, card.id);
	}

	for (const Status &status : statuses) {
		if (status.deleted) continue;
		markId(*this, marks, status.id);
	}

	for (const Prop &prop : props) {
		if (!marks.find(prop.id)) garbageIds.push(prop.id);
	}

	for (const Character &chr : characters) {
		if (!marks.find(chr.id)) garbageIds.push(chr.id);
	}

	for (const Card &card : cards) {
		if (!marks.find(card.id)) garbageIds.push(card.id);
	}

	for (const Status &status : statuses) {
		if (!marks.find(status.id)) garbageIds.push(status.id);
	}
}

static void markPrefab(const ServerState &state, sf::HashSet<sf::Symbol> &marks, const sf::Symbol &prefabName)
{
	walkPrefabs((ServerState&)state, nullptr, &marks, prefabName);
}

void ServerState::garbageCollectPrefabs(sf::Array<sf::Symbol> &garbagePrefabs) const
{
	sf::HashSet<sf::Symbol> marks;
	for (const Prop &prop : props) {
		markPrefab(*this, marks, prop.prefabName);
	}
	for (const Character &chr : characters) {
		markPrefab(*this, marks, chr.prefabName);
	}
	for (const Card &card : cards) {
		markPrefab(*this, marks, card.prefabName);
	}
	for (const auto &pair : charactersToSelect) {
		markPrefab(*this, marks, pair.key);
	}

	if (marks.size() < prefabs.size()) {
		garbagePrefabs.reserve(prefabs.size() - marks.size());
	}
	for (const Prefab &prefab : prefabs) {
		if (marks.find(prefab.name)) continue;
		garbagePrefabs.push(prefab.name);
	}
}

void ServerState::removeIds(sf::Slice<const uint32_t> ids)
{
	for (uint32_t id : ids) {
		switch (getIdType(id)) {
		case IdType::Prop: props.remove(id); break;
		case IdType::Character: characters.remove(id); break;
		case IdType::Card: cards.remove(id); break;
		case IdType::Status: statuses.remove(id); break;
		}
	}
}

void ServerState::removePrefabs(sf::Slice<const sf::Symbol> names)
{
	for (const sf::Symbol &name : names) {
		prefabs.remove(name);
	}
}

}


namespace sf {
using namespace sv;

template<> void initType<ServerState>(Type *t)
{
	static Field fields[] = {
		sf_field(ServerState, prefabs),
		sf_field(ServerState, props),
		sf_field(ServerState, characters),
		sf_field(ServerState, cards),
		sf_field(ServerState, statuses),
		sf_field(ServerState, charactersToSelect),
		sf_field(ServerState, nextIdByType),
	};
	sf_struct(t, ServerState, fields);
}

}