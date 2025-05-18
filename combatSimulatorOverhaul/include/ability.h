#pragma once

#include <string>
#include <vector>
#include <map>
#include <limits>
#include "Buff.h"
#include "trigger.h"
#include <nlohmann/json.hpp>

class Unit;

// Equivalent to JavaScript abilityEffect structure
struct AbilityEffect {
    std::string targetType;
    std::string effectType;
    std::string combatStyleHrid;
    std::string damageType;
    float damageFlat;
    float damageRatio;
    float bonusAccuracyRatio;
    float damageOverTimeRatio;
    int damageOverTimeDuration;
    float armorDamageRatio;
    float hpDrainRatio;
    float pierceChance;
    float blindChance;
    int blindDuration;
    float silenceChance;
    int silenceDuration;
    float stunChance;
    int stunDuration;
    float spendHpRatio;
    std::vector<Buff> buffs;
};

class Ability {
public:
    // Constructor that mirrors the JavaScript constructor
    Ability(const std::string& hrid, int level = 1, const std::vector<Trigger>* triggers = nullptr);
    
    // Factory method to create an instance from JSON data
    static Ability createFromDTO(const nlohmann::json& dto);
    
    // Determine if ability should activate
    bool shouldTrigger(float currentTime, const Unit& source, const Unit& target, 
                       const std::vector<Unit>& friendlies, const std::vector<Unit>& enemies);

    // Public properties
    std::string hrid;
    int level;
    int manaCost;
    float cooldownDuration;
    float castDuration;
    bool isSpecialAbility;
    std::vector<AbilityEffect> abilityEffects;
    std::vector<Trigger> triggers;
    float lastUsed;

private:
    // Helper methods to load ability data
    static std::map<std::string, nlohmann::json> loadAbilityDetailMap();
    static std::map<std::string, nlohmann::json> initializeHardcodedAbilities();
};
