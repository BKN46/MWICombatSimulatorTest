#include "consumable.h"
#include "combat_unit.h"
#include <limits>
#include <algorithm>

// Initialize the external map that will be loaded from JSON
std::unordered_map<std::string, ItemDetail> itemDetailMap;

Consumable::Consumable(const std::string& hrid, const std::vector<Trigger*>* triggers) : hrid(hrid), lastUsed(std::numeric_limits<int>::min()) {
    // Look up the consumable in the item detail map
    auto it = itemDetailMap.find(hrid);
    if (it == itemDetailMap.end()) {
        throw std::runtime_error("No consumable found for hrid: " + hrid);
    }
    
    const ItemDetail& gameConsumable = it->second;
    
    cooldownDuration = gameConsumable.consumableDetail.cooldownDuration;
    hitpointRestore = gameConsumable.consumableDetail.hitpointRestore;
    manapointRestore = gameConsumable.consumableDetail.manapointRestore;
    recoveryDuration = gameConsumable.consumableDetail.recoveryDuration;
    categoryHrid = gameConsumable.categoryHrid;
    
    // Create buffs
    for (const auto& consumableBuff : gameConsumable.consumableDetail.buffs) {
        buffs.push_back(new Buff(consumableBuff));
    }
    
    // Setup triggers
    if (triggers) {
        // Copy provided triggers
        for (const auto& trigger : *triggers) {
            this->triggers.push_back(new Trigger(*trigger));
        }
    } else {
        // Create default triggers
        for (const auto& defaultTrigger : gameConsumable.consumableDetail.defaultCombatTriggers) {
            triggers.push_back(new Trigger(
                defaultTrigger.dependencyHrid,
                defaultTrigger.conditionHrid,
                defaultTrigger.comparatorHrid,
                defaultTrigger.value
            ));
        }
    }
}

Consumable::~Consumable() {
    for (auto buff : buffs) {
        delete buff;
    }
    
    for (auto trigger : triggers) {
        delete trigger;
    }
}

Consumable* Consumable::createFromDTO(const ConsumableDTO& dto) {
    std::vector<Trigger*> triggers;
    for (const auto& triggerDto : dto.triggers) {
        triggers.push_back(&Trigger::createFromDTO(triggerDto));
    }
    
    Consumable* consumable = new Consumable(dto.hrid, &triggers);
    
    return consumable;
}

bool Consumable::shouldTrigger(int currentTime, CombatUnit* source, CombatUnit* target,
                               const std::vector<CombatUnit*>& friendlies, 
                               const std::vector<CombatUnit*>& enemies) {
    if (source->isStunned) {
        return false;
    }
    
    double consumableHaste;
    if (categoryHrid.find("food") != std::string::npos) {
        consumableHaste = source->combatDetails.combatStats.foodHaste;
    } else {
        consumableHaste = source->combatDetails.combatStats.drinkConcentration;
    }
    
    int cooldownDuration = this->cooldownDuration;
    if (consumableHaste > 0) {
        cooldownDuration = cooldownDuration / (1 + consumableHaste);
    }
    
    if (lastUsed + cooldownDuration > currentTime) {
        return false;
    }
    
    if (triggers.empty()) {
        return true;
    }
    
    bool shouldTrigger = true;
    for (const auto& trigger : triggers) {
        if (!trigger->isActive(source, target, friendlies, enemies, currentTime)) {
            shouldTrigger = false;
            break;
        }
    }
    
    return shouldTrigger;
}
