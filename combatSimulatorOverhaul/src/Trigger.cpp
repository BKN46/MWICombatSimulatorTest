#include "trigger.h"
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <variant>
#include "CombatTriggerDependencyMap.h" // Assuming we'll create this from the JSON

Trigger::Trigger(const std::string& dependencyHrid, 
                 const std::string& conditionHrid, 
                 const std::string& comparatorHrid, 
                 double value)
    : dependencyHrid(dependencyHrid),
      conditionHrid(conditionHrid),
      comparatorHrid(comparatorHrid),
      value(value) {
}

std::shared_ptr<Trigger> Trigger::createFromDTO(const std::unordered_map<std::string, std::variant<std::string, double>>& dto) {
    std::string depHrid = std::get<std::string>(dto.at("dependencyHrid"));
    std::string condHrid = std::get<std::string>(dto.at("conditionHrid"));
    std::string compHrid = std::get<std::string>(dto.at("comparatorHrid"));
    double val = std::get<double>(dto.at("value"));
    
    return std::make_shared<Trigger>(depHrid, condHrid, compHrid, val);
}

bool Trigger::isActive(const std::shared_ptr<CombatUnit>& source, 
                      const std::shared_ptr<CombatUnit>& target,
                      const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                      const std::vector<std::shared_ptr<CombatUnit>>& enemies,
                      double currentTime) {
    if (CombatTriggerDependencyMap::get().at(dependencyHrid).isSingleTarget) {
        return isActiveSingleTarget(source, target, currentTime);
    } else {
        return isActiveMultiTarget(friendlies, enemies, currentTime);
    }
}

bool Trigger::isActiveSingleTarget(const std::shared_ptr<CombatUnit>& source, 
                                  const std::shared_ptr<CombatUnit>& target,
                                  double currentTime) {
    double dependencyValue;
    if (dependencyHrid == "/combat_trigger_dependencies/self") {
        dependencyValue = getDependencyValue(source, currentTime);
    }
    else if (dependencyHrid == "/combat_trigger_dependencies/targeted_enemy") {
        if (!target) {
            return false;
        }
        dependencyValue = getDependencyValue(target, currentTime);
    }
    else {
        throw std::runtime_error("Unknown dependencyHrid in trigger: " + dependencyHrid);
    }

    return compareValue(dependencyValue);
}

bool Trigger::isActiveMultiTarget(const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                                 const std::vector<std::shared_ptr<CombatUnit>>& enemies,
                                 double currentTime) {
    std::vector<std::shared_ptr<CombatUnit>> dependency;
    
    if (dependencyHrid == "/combat_trigger_dependencies/all_allies") {
        dependency = friendlies;
    }
    else if (dependencyHrid == "/combat_trigger_dependencies/all_enemies") {
        if (enemies.empty()) {
            return false;
        }
        dependency = enemies;
    }
    else {
        throw std::runtime_error("Unknown dependencyHrid in trigger: " + dependencyHrid);
    }

    double dependencyValue = 0.0;
    if (conditionHrid == "/combat_trigger_conditions/number_of_active_units") {
        dependencyValue = std::count_if(dependency.begin(), dependency.end(), 
            [](const std::shared_ptr<CombatUnit>& unit) { 
                return unit->combatDetails.currentHitpoints > 0; 
            });
    }
    else if (conditionHrid == "/combat_trigger_conditions/number_of_dead_units") {
        dependencyValue = std::count_if(dependency.begin(), dependency.end(), 
            [](const std::shared_ptr<CombatUnit>& unit) { 
                return unit->combatDetails.currentHitpoints <= 0; 
            });
    }
    else if (conditionHrid == "/combat_trigger_conditions/lowest_hp_percentage") {
        double lowestHpPercentage = std::numeric_limits<double>::max();
        for (const auto& unit : dependency) {
            double currentHpPercentage = unit->combatDetails.currentHitpoints / unit->combatDetails.maxHitpoints;
            lowestHpPercentage = std::min(lowestHpPercentage, currentHpPercentage);
        }
        dependencyValue = lowestHpPercentage * 100;
    }
    else {
        // Sum up dependency values for all units
        for (const auto& unit : dependency) {
            dependencyValue += getDependencyValue(unit, currentTime);
        }
    }

    return compareValue(dependencyValue);
}

double Trigger::getDependencyValue(const std::shared_ptr<CombatUnit>& source, double currentTime) {
    // Check if it's a buff condition
    if (conditionHrid.find("/combat_trigger_conditions/") == 0) {
        // Buffs list
        static const std::vector<std::string> buffConditions = {
            "/combat_trigger_conditions/berserk",
            "/combat_trigger_conditions/elemental_affinity_fire_amplify",
            "/combat_trigger_conditions/elemental_affinity_nature_amplify",
            // ... all the other buff condition types from the original file
            "/combat_trigger_conditions/fury_damage"
        };

        // Check if it's a buff
        auto it = std::find(buffConditions.begin(), buffConditions.end(), conditionHrid);
        if (it != buffConditions.end()) {
            std::string buffHrid = "/buff_uniques";
            buffHrid += conditionHrid.substr(conditionHrid.find_last_of('/'));
            return source->combatBuffs[buffHrid];
        }
    }

    // Handle specific conditions
    if (conditionHrid == "/combat_trigger_conditions/current_hp") {
        return source->combatDetails.currentHitpoints;
    }
    else if (conditionHrid == "/combat_trigger_conditions/current_mp") {
        return source->combatDetails.currentManapoints;
    }
    else if (conditionHrid == "/combat_trigger_conditions/missing_hp") {
        return source->combatDetails.maxHitpoints - source->combatDetails.currentHitpoints;
    }
    else if (conditionHrid == "/combat_trigger_conditions/missing_mp") {
        return source->combatDetails.maxManapoints - source->combatDetails.currentManapoints;
    }
    else if (conditionHrid == "/combat_trigger_conditions/stun_status") {
        // Replicate the game's behaviour of "stun status active" triggers activating
        // immediately after the stun has worn off
        return source->isStunned || source->stunExpireTime == currentTime;
    }
    else if (conditionHrid == "/combat_trigger_conditions/blind_status") {
        return source->isBlinded || source->blindExpireTime == currentTime;
    }
    else if (conditionHrid == "/combat_trigger_conditions/silence_status") {
        return source->isSilenced || source->silenceExpireTime == currentTime;
    }
    else if (conditionHrid == "/combat_trigger_conditions/curse") {
        return source->curseValue > 0;
    }
    else if (conditionHrid == "/combat_trigger_conditions/weaken") {
        return source->isWeakened || source->weakenExpireTime == currentTime;
    }
    else {
        throw std::runtime_error("Unknown conditionHrid in trigger: " + conditionHrid);
    }
}

bool Trigger::compareValue(double dependencyValue) {
    if (comparatorHrid == "/combat_trigger_comparators/greater_than_equal") {
        return dependencyValue >= value;
    }
    else if (comparatorHrid == "/combat_trigger_comparators/less_than_equal") {
        return dependencyValue <= value;
    }
    else if (comparatorHrid == "/combat_trigger_comparators/is_active") {
        return dependencyValue != 0;
    }
    else if (comparatorHrid == "/combat_trigger_comparators/is_inactive") {
        return dependencyValue == 0;
    }
    else {
        throw std::runtime_error("Unknown comparatorHrid in trigger: " + comparatorHrid);
    }
}