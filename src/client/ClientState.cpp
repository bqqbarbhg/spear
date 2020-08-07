#include "ClientState.h"

namespace cl {

#if 0

void ClientState::applyEvent(const sv::Event &event, bool simulated)
{
	if (auto *e = event.as<sv::CardCooldownTickEvent>()) {
		if (Card *c = cards.find(e->cardId)) {
			if (c->cooldownLeft > 0) {
				c->cooldownLeft -= 1;
				c->cooldownAnim = 1.0f;
			}
		}
	} else if (auto *e = event.as<sv::StatusAddEvent>()) {
		if (Prefab *prefab = prefabs.find(e->status.prefabName)) {
			if (auto *statusComp = prefab->s.findComponent<sv::StatusComponent>()) {
				if (statusComp->startEffect && !simulated) {
					spawnEffect(statusComp->startEffect, e->status.id);
				}
				if (statusComp->activeEffect && !simulated) {
					spawnEffect(statusComp->activeEffect, e->status.id);
				}
			}
		}
	} else if (auto *e = event.as<sv::StatusTickEvent>()) {
	} else if (auto *e = event.as<sv::StatusRemoveEvent>()) {
	} else if (auto *e = event.as<sv::DamageEvent>()) {
	} else if (auto *e = event.as<sv::LoadPrefabEvent>()) {
	} else if (auto *e = event.as<sv::RemoveGarbageIdsEvent>()) {
	} else if (auto *e = event.as<sv::RemoveGarbagePrefabsEvent>()) {
	} else if (auto *e = event.as<sv::AddPropEvent>()) {
	} else if (auto *e = event.as<sv::AddCharacterEvent>()) {
	} else if (auto *e = event.as<sv::AddCardEvent>()) {
	} else if (auto *e = event.as<sv::GiveCardEvent>()) {
	} else if (auto *e = event.as<sv::SelectCardEvent>()) {
	} else if (auto *e = event.as<sv::AddCharacterToSpawn>()) {
	} else if (auto *e = event.as<sv::SelectCharacterToSpawnEvent>()) {
	}
}

#endif

void ClientState::renderShadows(const RenderShadowArgs &args)
{
}

}
