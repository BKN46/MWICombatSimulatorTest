#include "Ability.h"
#include "Unit.h"
#include <fstream>
#include <stdexcept>

// Load ability details from JSON file
std::map<std::string, nlohmann::json> Ability::loadAbilityDetailMap() {
    std::map<std::string, nlohmann::json> abilityDetailMap;
    try {
        std::ifstream file("data/abilityDetailMap.json");
        if (file.is_open()) {
            nlohmann::json jsonData;
            file >> jsonData;
            for (auto& [key, value] : jsonData.items()) {
                abilityDetailMap[key] = value;
            }
        }
    }
    catch (const std::exception& e) {
        // Handle exception
    }
    return abilityDetailMap;
}

// Initialize hardcoded abilities (blaze and bloom) equivalent to JavaScript version
std::map<std::string, nlohmann::json> Ability::initializeHardcodedAbilities() {
    std::map<std::string, nlohmann::json> abilities;
    
    // Blaze ability
    nlohmann::json blaze;
    blaze["hrid"] = "/abilities/blaze";
    blaze["name"] = "Blaze";
    blaze["description"] = "";
    blaze["isSpecialAbility"] = false;
    blaze["manaCost"] = 0;
    blaze["cooldownDuration"] = 0;
    blaze["castDuration"] = 0;
    
    nlohmann::json blazeEffect;
    blazeEffect["targetType"] = "allEnemies";
    blazeEffect["effectType"] = "/ability_effect_types/damage";
    blazeEffect["combatStyleHrid"] = "/combat_styles/magic";
    blazeEffect["damageType"] = "/damage_types/fire";
    blazeEffect["baseDamageFlat"] = 0;
    blazeEffect["baseDamageFlatLevelBonus"] = 0.0;
    blazeEffect["baseDamageRatio"] = 0.3;
    blazeEffect["baseDamageRatioLevelBonus"] = 0;
    blazeEffect["bonusAccuracyRatio"] = 0;
    blazeEffect["bonusAccuracyRatioLevelBonus"] = 0;
    blazeEffect["damageOverTimeRatio"] = 0;
    blazeEffect["damageOverTimeDuration"] = 0;
    blazeEffect["armorDamageRatio"] = 0;
    blazeEffect["armorDamageRatioLevelBonus"] = 0;
    blazeEffect["hpDrainRatio"] = 0;
    blazeEffect["pierceChance"] = 0;
    blazeEffect["blindChance"] = 0;
    blazeEffect["blindDuration"] = 0;
    blazeEffect["silenceChance"] = 0;
    blazeEffect["silenceDuration"] = 0;
    blazeEffect["stunChance"] = 0;
    blazeEffect["stunDuration"] = 0;
    blazeEffect["spendHpRatio"] = 0;
    blazeEffect["buffs"] = nullptr;
    
    nlohmann::json blazeTrigger1;
    blazeTrigger1["dependencyHrid"] = "/combat_trigger_dependencies/all_enemies";
    blazeTrigger1["conditionHrid"] = "/combat_trigger_conditions/number_of_active_units";
    blazeTrigger1["comparatorHrid"] = "/combat_trigger_comparators/greater_than_equal";
    blazeTrigger1["value"] = 1;
    
    nlohmann::json blazeTrigger2;
    blazeTrigger2["dependencyHrid"] = "/combat_trigger_dependencies/all_enemies";
    blazeTrigger2["conditionHrid"] = "/combat_trigger_conditions/current_hp";
    blazeTrigger2["comparatorHrid"] = "/combat_trigger_comparators/greater_than_equal";
    blazeTrigger2["value"] = 1;
    
    blaze["abilityEffects"] = nlohmann::json::array({blazeEffect});
    blaze["defaultCombatTriggers"] = nlohmann::json::array({blazeTrigger1, blazeTrigger2});
    
    // Bloom ability
    nlohmann::json bloom;
    bloom["hrid"] = "/abilities/bloom";
    bloom["name"] = "Bloom";
    bloom["description"] = "";
    bloom["isSpecialAbility"] = false;
    bloom["manaCost"] = 0;
    bloom["cooldownDuration"] = 0;
    bloom["castDuration"] = 0;
    
    nlohmann::json bloomEffect;
    bloomEffect["targetType"] = "lowestHpAlly";
    bloomEffect["effectType"] = "/ability_effect_types/heal";
    bloomEffect["combatStyleHrid"] = "/combat_styles/magic";
    bloomEffect["damageType"] = "";
    bloomEffect["baseDamageFlat"] = 10;
    bloomEffect["baseDamageFlatLevelBonus"] = 0;
    bloomEffect["baseDamageRatio"] = 0.15;
    bloomEffect["baseDamageRatioLevelBonus"] = 0;
    bloomEffect["bonusAccuracyRatio"] = 0;
    bloomEffect["bonusAccuracyRatioLevelBonus"] = 0;
    bloomEffect["damageOverTimeRatio"] = 0;
    bloomEffect["damageOverTimeDuration"] = 0;
    bloomEffect["armorDamageRatio"] = 0;
    bloomEffect["armorDamageRatioLevelBonus"] = 0;
    bloomEffect["hpDrainRatio"] = 0;
    bloomEffect["pierceChance"] = 0;
    bloomEffect["blindChance"] = 0;
    bloomEffect["blindDuration"] = 0;
    bloomEffect["silenceChance"] = 0;
    bloomEffect["silenceDuration"] = 0;
    bloomEffect["stunChance"] = 0;
    bloomEffect["stunDuration"] = 0;
    bloomEffect["spendHpRatio"] = 0;
    bloomEffect["buffs"] = nullptr;
    
    nlohmann::json bloomTrigger;
    bloomTrigger["dependencyHrid"] = "/combat_trigger_dependencies/all_allies";
    bloomTrigger["conditionHrid"] = "/combat_trigger_conditions/lowest_hp_percentage";
    bloomTrigger["comparatorHrid"] = "/combat_trigger_comparators/less_than_equal";
    bloomTrigger["value"] = 100;
    
    bloom["abilityEffects"] = nlohmann::json::array({bloomEffect});
    bloom["defaultCombatTriggers"] = nlohmann::json::array({bloomTrigger});
    
    abilities["blaze"] = blaze;
    abilities["bloom"] = bloom;
    
    return abilities;
}

// Constructor implementation
Ability::Ability(const std::string& hrid, int level, const std::vector<Trigger>* triggers) 
    : hrid(hrid), level(level), lastUsed(std::numeric_limits<float>::lowest()) {
    
    static std::map<std::string, nlohmann::json> abilityDetailMap = loadAbilityDetailMap();
    static std::map<std::string, nlohmann::json> abilityFromCombatStat = initializeHardcodedAbilities();
    
    nlohmann::json gameAbility;
    
    if (abilityDetailMap.find(hrid) != abilityDetailMap.end()) {
        gameAbility = abilityDetailMap[hrid];
    } else if (abilityFromCombatStat.find(hrid) != abilityFromCombatStat.end()) {
        gameAbility = abilityFromCombatStat[hrid];
    } else {
        throw std::runtime_error("No ability found for hrid: " + hrid);
    }
    
    manaCost = gameAbility["manaCost"];
    cooldownDuration = gameAbility["cooldownDuration"];
    castDuration = gameAbility["castDuration"];
    isSpecialAbility = gameAbility["isSpecialAbility"];
    
    // Process ability effects
    for (const auto& effect : gameAbility["abilityEffects"]) {
        AbilityEffect abilityEffect;
        abilityEffect.targetType = effect["targetType"];
        abilityEffect.effectType = effect["effectType"];
        abilityEffect.combatStyleHrid = effect["combatStyleHrid"];
        abilityEffect.damageType = effect["damageType"];
        abilityEffect.damageFlat = effect["baseDamageFlat"].get<float>() + 
                                  (level - 1) * effect["baseDamageFlatLevelBonus"].get<float>();
        abilityEffect.damageRatio = effect["baseDamageRatio"].get<float>() + 
                                   (level - 1) * effect["baseDamageRatioLevelBonus"].get<float>();
        abilityEffect.bonusAccuracyRatio = effect["bonusAccuracyRatio"].get<float>() + 
                                          (level - 1) * effect["bonusAccuracyRatioLevelBonus"].get<float>();
        abilityEffect.damageOverTimeRatio = effect["damageOverTimeRatio"];
        abilityEffect.damageOverTimeDuration = effect["damageOverTimeDuration"];
        abilityEffect.armorDamageRatio = effect["armorDamageRatio"].get<float>() + 
                                        (level - 1) * effect["armorDamageRatioLevelBonus"].get<float>();
        abilityEffect.hpDrainRatio = effect["hpDrainRatio"];
        abilityEffect.pierceChance = effect["pierceChance"];
        abilityEffect.blindChance = effect["blindChance"];
        abilityEffect.blindDuration = effect["blindDuration"];
        abilityEffect.silenceChance = effect["silenceChance"];
        abilityEffect.silenceDuration = effect["silenceDuration"];
        abilityEffect.stunChance = effect["stunChance"];
        abilityEffect.stunDuration = effect["stunDuration"];
        abilityEffect.spendHpRatio = effect["spendHpRatio"];
        
        // Process buffs if they exist
        if (!effect["buffs"].is_null()) {
            for (const auto& buff : effect["buffs"]) {
                abilityEffect.buffs.push_back(Buff(buff, level));
            }
        }
        
        abilityEffects.push_back(abilityEffect);
    }
    
    // Process triggers
    if (triggers) {
        this->triggers = *triggers;
    } else {
        for (const auto& defaultTrigger : gameAbility["defaultCombatTriggers"]) {
            Trigger trigger(
                defaultTrigger["dependencyHrid"],
                defaultTrigger["conditionHrid"],
                defaultTrigger["comparatorHrid"],
                defaultTrigger["value"]
            );
            this->triggers.push_back(trigger);
        }
    }
}

// Static factory method implementation
Ability Ability::createFromDTO(const nlohmann::json& dto) {
    std::vector<Trigger> triggers;
    for (const auto& triggerDTO : dto["triggers"]) {
        triggers.push_back(Trigger::createFromDTO(triggerDTO));
    }
    
    return Ability(dto["hrid"], dto["level"], &triggers);
}

// Method to check if the ability should trigger
bool Ability::shouldTrigger(float currentTime, const Unit& source, const Unit& target, 
                           const std::vector<Unit>& friendlies, const std::vector<Unit>& enemies) {
    if (source.isStunned) {
        return false;
    }
    
    if (source.isSilenced) {
        return false;
    }
    
    float haste = source.combatDetails.combatStats.abilityHaste;
    float currentCooldownDuration = cooldownDuration;
    if (haste > 0) {
        currentCooldownDuration = currentCooldownDuration * 100.0f / (100.0f + haste);
    }
    
    if (lastUsed + currentCooldownDuration > currentTime) {
        return false;
    }
    
    if (triggers.empty()) {
        return true;
    }
    
    bool shouldTrigger = true;
    for (const auto& trigger : triggers) {
        if (!trigger.isActive(source, target, friendlies, enemies, currentTime)) {
            shouldTrigger = false;
            break;
        }
    }
    
    return shouldTrigger;
}
