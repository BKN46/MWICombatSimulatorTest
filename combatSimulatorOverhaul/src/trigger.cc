// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the Trigger class for the combat simulator.

#include "trigger.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "combat_unit.h"
#include "nlohmann/json.hpp"

namespace {
// Helper function to load the combat trigger dependency detail map
nlohmann::json LoadCombatTriggerDependencyDetailMap() {
  // In a real implementation, this would load from a file or resource
  // For now, we'll use a placeholder
  static nlohmann::json map = nullptr;
  if (map == nullptr) {
    // In production code, load this from the actual file
    // map = nlohmann::json::parse(std::ifstream("data/combatTriggerDependencyDetailMap.json"));
    
    // For this example, we'll create a minimal version of the map
    map = nlohmann::json::object();
    map["/combat_trigger_dependencies/self"]["isSingleTarget"] = true;
    map["/combat_trigger_dependencies/targeted_enemy"]["isSingleTarget"] = true;
    map["/combat_trigger_dependencies/all_allies"]["isSingleTarget"] = false;
    map["/combat_trigger_dependencies/all_enemies"]["isSingleTarget"] = false;
  }
  return map;
}
}  // namespace

namespace combat_simulator {

Trigger::Trigger(const std::string& dependency_hrid, 
                const std::string& condition_hrid,
                const std::string& comparator_hrid,
                double value)
    : dependency_hrid_(dependency_hrid),
      condition_hrid_(condition_hrid),
      comparator_hrid_(comparator_hrid),
      value_(value) {}

std::unique_ptr<Trigger> Trigger::CreateFromDTO(const nlohmann::json& dto) {
  return std::make_unique<Trigger>(
      dto["dependencyHrid"],
      dto["conditionHrid"],
      dto["comparatorHrid"],
      dto.value("value", 0.0));
}

bool Trigger::IsActive(const CombatUnit& source, 
                      const CombatUnit* target,
                      const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                      const std::vector<std::shared_ptr<CombatUnit>>& enemies,
                      int64_t current_time) const {
  const auto& dependency_map = LoadCombatTriggerDependencyDetailMap();
  
  bool is_single_target = dependency_map[dependency_hrid_]["isSingleTarget"];
  if (is_single_target) {
    return IsActiveSingleTarget(source, target, current_time);
  } else {
    return IsActiveMultiTarget(friendlies, enemies, current_time);
  }
}

bool Trigger::IsActiveSingleTarget(const CombatUnit& source,
                                 const CombatUnit* target,
                                 int64_t current_time) const {
  double dependency_value;
  
  if (dependency_hrid_ == "/combat_trigger_dependencies/self") {
    dependency_value = GetDependencyValue(source, current_time);
  } else if (dependency_hrid_ == "/combat_trigger_dependencies/targeted_enemy") {
    if (!target) {
      return false;
    }
    dependency_value = GetDependencyValue(*target, current_time);
  } else {
    throw std::runtime_error("Unknown dependencyHrid in trigger: " + dependency_hrid_);
  }
  
  return CompareValue(dependency_value);
}

bool Trigger::IsActiveMultiTarget(const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                                const std::vector<std::shared_ptr<CombatUnit>>& enemies,
                                int64_t current_time) const {
  const std::vector<std::shared_ptr<CombatUnit>>* dependency = nullptr;
  
  if (dependency_hrid_ == "/combat_trigger_dependencies/all_allies") {
    dependency = &friendlies;
  } else if (dependency_hrid_ == "/combat_trigger_dependencies/all_enemies") {
    if (enemies.empty()) {
      return false;
    }
    dependency = &enemies;
  } else {
    throw std::runtime_error("Unknown dependencyHrid in trigger: " + dependency_hrid_);
  }
  
  double dependency_value;
  
  if (condition_hrid_ == "/combat_trigger_conditions/number_of_active_units") {
    dependency_value = std::count_if(dependency->begin(), dependency->end(), 
                                   [](const auto& unit) { 
                                     return unit->combat_details().current_hitpoints > 0; 
                                   });
  } else if (condition_hrid_ == "/combat_trigger_conditions/number_of_dead_units") {
    dependency_value = std::count_if(dependency->begin(), dependency->end(), 
                                   [](const auto& unit) { 
                                     return unit->combat_details().current_hitpoints <= 0; 
                                   });
  } else if (condition_hrid_ == "/combat_trigger_conditions/lowest_hp_percentage") {
    dependency_value = 200.0;  // Start with a value higher than 100%
    for (const auto& unit : *dependency) {
      double current_hp_percentage = unit->combat_details().current_hitpoints / 
                                    unit->combat_details().max_hitpoints;
      dependency_value = std::min(dependency_value, current_hp_percentage);
    }
    dependency_value *= 100.0;  // Convert to percentage
  } else {
    // For other conditions, sum up the dependency values of all units
    dependency_value = 0.0;
    for (const auto& unit : *dependency) {
      dependency_value += GetDependencyValue(*unit, current_time);
    }
  }
  
  return CompareValue(dependency_value);
}

double Trigger::GetDependencyValue(const CombatUnit& source, int64_t current_time) const {
  // Handle buff conditions
  if (condition_hrid_.find("/combat_trigger_conditions/") == 0 &&
     (condition_hrid_.find("_coffee") != std::string::npos ||
      condition_hrid_.find("_aura") != std::string::npos ||
      condition_hrid_.find("berserk") != std::string::npos ||
      condition_hrid_.find("elemental_affinity") != std::string::npos ||
      condition_hrid_.find("frenzy") != std::string::npos ||
      condition_hrid_.find("precision") != std::string::npos ||
      condition_hrid_.find("spike_shell") != std::string::npos ||
      condition_hrid_.find("toughness") != std::string::npos ||
      condition_hrid_.find("vampirism") != std::string::npos ||
      condition_hrid_.find("ice_spear") != std::string::npos ||
      condition_hrid_.find("toxic_pollen") != std::string::npos ||
      condition_hrid_.find("puncture") != std::string::npos ||
      condition_hrid_.find("frost_surge") != std::string::npos ||
      condition_hrid_.find("elusiveness") != std::string::npos ||
      condition_hrid_.find("insanity") != std::string::npos ||
      condition_hrid_.find("invincible") != std::string::npos ||
      condition_hrid_.find("provoke") != std::string::npos ||
      condition_hrid_.find("taunt") != std::string::npos ||
      condition_hrid_.find("crippling_slash") != std::string::npos ||
      condition_hrid_.find("mana_spring") != std::string::npos ||
      condition_hrid_.find("pestilent_shot") != std::string::npos ||
      condition_hrid_.find("smoke_burst") != std::string::npos ||
      condition_hrid_.find("arcane_reflection") != std::string::npos ||
      condition_hrid_.find("fracturing_impact") != std::string::npos ||
      condition_hrid_.find("maim") != std::string::npos ||
      condition_hrid_.find("fury") != std::string::npos)) {
    
    std::string buff_hrid = "/buff_uniques";
    size_t last_slash = condition_hrid_.find_last_of('/');
    if (last_slash != std::string::npos) {
      buff_hrid += condition_hrid_.substr(last_slash);
    }
    
    // Check if the buff exists in the unit's combat buffs
    const auto& combat_buffs = source.combat_buffs();
    auto it = combat_buffs.find(buff_hrid);
    if (it != combat_buffs.end()) {
      return 1.0;  // Buff is active
    }
    return 0.0;  // Buff is not active
  }
  
  // Handle other conditions
  if (condition_hrid_ == "/combat_trigger_conditions/current_hp") {
    return source.combat_details().current_hitpoints;
  } else if (condition_hrid_ == "/combat_trigger_conditions/current_mp") {
    return source.combat_details().current_manapoints;
  } else if (condition_hrid_ == "/combat_trigger_conditions/missing_hp") {
    return source.combat_details().max_hitpoints - source.combat_details().current_hitpoints;
  } else if (condition_hrid_ == "/combat_trigger_conditions/missing_mp") {
    return source.combat_details().max_manapoints - source.combat_details().current_manapoints;
  } else if (condition_hrid_ == "/combat_trigger_conditions/stun_status") {
    // Replicate the game's behavior of "stun status active" triggers activating
    // immediately after the stun has worn off
    return (source.is_stunned() || source.stun_expire_time() == current_time) ? 1.0 : 0.0;
  } else if (condition_hrid_ == "/combat_trigger_conditions/blind_status") {
    return (source.is_blinded() || source.blind_expire_time() == current_time) ? 1.0 : 0.0;
  } else if (condition_hrid_ == "/combat_trigger_conditions/silence_status") {
    return (source.is_silenced() || source.silence_expire_time() == current_time) ? 1.0 : 0.0;
  } else if (condition_hrid_ == "/combat_trigger_conditions/curse") {
    return source.curse_value() > 0 ? 1.0 : 0.0;
  } else if (condition_hrid_ == "/combat_trigger_conditions/weaken") {
    return (source.is_weakened() || source.weaken_expire_time() == current_time) ? 1.0 : 0.0;
  }
  
  throw std::runtime_error("Unknown conditionHrid in trigger: " + condition_hrid_);
}

bool Trigger::CompareValue(double dependency_value) const {
  if (comparator_hrid_ == "/combat_trigger_comparators/greater_than_equal") {
    return dependency_value >= value_;
  } else if (comparator_hrid_ == "/combat_trigger_comparators/less_than_equal") {
    return dependency_value <= value_;
  } else if (comparator_hrid_ == "/combat_trigger_comparators/is_active") {
    return dependency_value != 0.0;
  } else if (comparator_hrid_ == "/combat_trigger_comparators/is_inactive") {
    return dependency_value == 0.0;
  }
  
  throw std::runtime_error("Unknown comparatorHrid in trigger: " + comparator_hrid_);
}

}  // namespace combat_simulator
