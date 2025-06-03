// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the Ability class for the combat simulator.

#include "ability.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "buff.h"
#include "combat_unit.h"
#include "trigger.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Static helper function to load ability detail map
nlohmann::json Ability::LoadAbilityDetailMap() {
  static nlohmann::json ability_detail_map = nullptr;
  if (ability_detail_map == nullptr) {
    try {
      std::ifstream file("combatSimulatorOverhaul/data/abilityDetailMap.json");
      if (file.is_open()) {
        ability_detail_map = nlohmann::json::parse(file);
      } else {
        // Fallback if file not found
        ability_detail_map = nlohmann::json::object();
      }
    } catch (const std::exception& e) {
      // Handle parsing errors
      ability_detail_map = nlohmann::json::object();
    }
  }
  return ability_detail_map;
}

// Static helper function to load abilities from combat stats
nlohmann::json Ability::LoadAbilityFromCombatStat() {
  static nlohmann::json abilities_from_combat_stat = nullptr;
  if (abilities_from_combat_stat == nullptr) {
    // Initialize with the hardcoded abilities from JS
    abilities_from_combat_stat = {
      {"blaze", {
        {"hrid", "/abilities/blaze"},
        {"name", "Blaze"},
        {"description", ""},
        {"isSpecialAbility", false},
        {"manaCost", 0},
        {"cooldownDuration", 0},
        {"castDuration", 0},
        {"abilityEffects", {{
          {"targetType", "allEnemies"},
          {"effectType", "/ability_effect_types/damage"},
          {"combatStyleHrid", "/combat_styles/magic"},
          {"damageType", "/damage_types/fire"},
          {"baseDamageFlat", 0},
          {"baseDamageFlatLevelBonus", 0.0},
          {"baseDamageRatio", 0.3},
          {"baseDamageRatioLevelBonus", 0},
          {"bonusAccuracyRatio", 0},
          {"bonusAccuracyRatioLevelBonus", 0},
          {"damageOverTimeRatio", 0},
          {"damageOverTimeDuration", 0},
          {"armorDamageRatio", 0},
          {"armorDamageRatioLevelBonus", 0},
          {"hpDrainRatio", 0},
          {"pierceChance", 0},
          {"blindChance", 0},
          {"blindDuration", 0},
          {"silenceChance", 0},
          {"silenceDuration", 0},
          {"stunChance", 0},
          {"stunDuration", 0},
          {"spendHpRatio", 0},
          {"buffs", nullptr}
        }}},
        {"defaultCombatTriggers", {{
          {"dependencyHrid", "/combat_trigger_dependencies/all_enemies"},
          {"conditionHrid", "/combat_trigger_conditions/number_of_active_units"},
          {"comparatorHrid", "/combat_trigger_comparators/greater_than_equal"},
          {"value", 1}
        }, {
          {"dependencyHrid", "/combat_trigger_dependencies/all_enemies"},
          {"conditionHrid", "/combat_trigger_conditions/current_hp"},
          {"comparatorHrid", "/combat_trigger_comparators/greater_than_equal"},
          {"value", 1}
        }}}
      }},
      {"bloom", {
        {"hrid", "/abilities/bloom"},
        {"name", "Bloom"},
        {"description", ""},
        {"isSpecialAbility", false},
        {"manaCost", 0},
        {"cooldownDuration", 0},
        {"castDuration", 0},
        {"abilityEffects", {{
          {"targetType", "lowestHpAlly"},
          {"effectType", "/ability_effect_types/heal"},
          {"combatStyleHrid", "/combat_styles/magic"},
          {"damageType", ""},
          {"baseDamageFlat", 10},
          {"baseDamageFlatLevelBonus", 0},
          {"baseDamageRatio", 0.15},
          {"baseDamageRatioLevelBonus", 0},
          {"bonusAccuracyRatio", 0},
          {"bonusAccuracyRatioLevelBonus", 0},
          {"damageOverTimeRatio", 0},
          {"damageOverTimeDuration", 0},
          {"armorDamageRatio", 0},
          {"armorDamageRatioLevelBonus", 0},
          {"hpDrainRatio", 0},
          {"pierceChance", 0},
          {"blindChance", 0},
          {"blindDuration", 0},
          {"silenceChance", 0},
          {"silenceDuration", 0},
          {"stunChance", 0},
          {"stunDuration", 0},
          {"spendHpRatio", 0},
          {"buffs", nullptr}
        }}},
        {"defaultCombatTriggers", {{
          {"dependencyHrid", "/combat_trigger_dependencies/all_allies"},
          {"conditionHrid", "/combat_trigger_conditions/lowest_hp_percentage"},
          {"comparatorHrid", "/combat_trigger_comparators/less_than_equal"},
          {"value", 100}
        }}}
      }}
    };
  }
  return abilities_from_combat_stat;
}

Ability::Ability(const std::string& hrid, int level,
                const std::vector<std::shared_ptr<Trigger>>& triggers) 
    : hrid_(hrid), level_(level) {
  
  // Load ability data from the appropriate source
  nlohmann::json game_ability;
  auto ability_detail_map = LoadAbilityDetailMap();
  
  // Try to find in the primary ability detail map
  if (ability_detail_map.contains(hrid)) {
    game_ability = ability_detail_map[hrid];
  } else {
    // Try to find in the combat stat abilities
    auto ability_from_combat_stat = LoadAbilityFromCombatStat();
    if (ability_from_combat_stat.contains(hrid.substr(hrid.find_last_of('/') + 1))) {
      game_ability = ability_from_combat_stat[hrid.substr(hrid.find_last_of('/') + 1)];
    } else {
      throw std::runtime_error("No ability found for hrid: " + hrid);
    }
  }
  
  // Set basic ability properties
  mana_cost_ = game_ability["manaCost"];
  cooldown_duration_ = game_ability["cooldownDuration"];
  cast_duration_ = game_ability["castDuration"];
  is_special_ability_ = game_ability["isSpecialAbility"];
  
  // Process ability effects
  for (const auto& effect : game_ability["abilityEffects"]) {
    AbilityEffect ability_effect;
    
    ability_effect.target_type = effect["targetType"];
    ability_effect.effect_type = effect["effectType"];
    ability_effect.combat_style_hrid = effect["combatStyleHrid"];
    ability_effect.damage_type = effect["damageType"];
    
    // Apply level scaling to damage values
    ability_effect.damage_flat = effect["baseDamageFlat"].get<double>() + 
                               (level_ - 1) * effect["baseDamageFlatLevelBonus"].get<double>();
    ability_effect.damage_ratio = effect["baseDamageRatio"].get<double>() + 
                               (level_ - 1) * effect["baseDamageRatioLevelBonus"].get<double>();
    ability_effect.bonus_accuracy_ratio = effect["bonusAccuracyRatio"].get<double>() + 
                                       (level_ - 1) * effect["bonusAccuracyRatioLevelBonus"].get<double>();
    
    // Set other ability effect properties
    ability_effect.damage_over_time_ratio = effect["damageOverTimeRatio"];
    ability_effect.damage_over_time_duration = effect["damageOverTimeDuration"];
    ability_effect.armor_damage_ratio = effect["armorDamageRatio"].get<double>() + 
                                     (level_ - 1) * effect["armorDamageRatioLevelBonus"].get<double>();
    ability_effect.hp_drain_ratio = effect["hpDrainRatio"];
    ability_effect.pierce_chance = effect["pierceChance"];
    ability_effect.blind_chance = effect["blindChance"];
    ability_effect.blind_duration = effect["blindDuration"];
    ability_effect.silence_chance = effect["silenceChance"];
    ability_effect.silence_duration = effect["silenceDuration"];
    ability_effect.stun_chance = effect["stunChance"];
    ability_effect.stun_duration = effect["stunDuration"];
    ability_effect.spend_hp_ratio = effect["spendHpRatio"];
    
    // Process buffs if they exist
    if (!effect["buffs"].is_null()) {
      for (const auto& buff_json : effect["buffs"]) {
        // Create a new Buff with level scaling
        Buff buff;
        buff.set_unique_hrid(buff_json["uniqueHrid"]);
        buff.set_type_hrid(buff_json["typeHrid"]);
        buff.set_ratio_boost(buff_json["ratioBoost"].get<double>() + 
                          (level_ - 1) * buff_json["ratioBoostLevelBonus"].get<double>());
        buff.set_flat_boost(buff_json["flatBoost"].get<double>() + 
                         (level_ - 1) * buff_json["flatBoostLevelBonus"].get<double>());
        buff.set_duration(buff_json["duration"]);
        
        ability_effect.buffs.push_back(buff);
      }
    }
    
    ability_effects_.push_back(ability_effect);
  }
  
  // Set triggers
  if (!triggers.empty()) {
    triggers_ = triggers;
  } else {
    // Create triggers from default combat triggers
    for (const auto& trigger_json : game_ability["defaultCombatTriggers"]) {
      auto trigger = std::make_shared<Trigger>(
          trigger_json["dependencyHrid"],
          trigger_json["conditionHrid"],
          trigger_json["comparatorHrid"],
          trigger_json["value"]
      );
      triggers_.push_back(trigger);
    }
  }
  
  // Initialize last used time to minimum value
  last_used_ = std::numeric_limits<int64_t>::min();
}

std::unique_ptr<Ability> Ability::CreateFromDTO(const nlohmann::json& dto) {
  std::vector<std::shared_ptr<Trigger>> triggers;
  
  // Create triggers from DTO
  for (const auto& trigger_json : dto["triggers"]) {
    triggers.push_back(Trigger::CreateFromDTO(trigger_json));
  }
  
  // Create and return a new Ability
  return std::make_unique<Ability>(
      dto["hrid"],
      dto["level"],
      triggers
  );
}

bool Ability::ShouldTrigger(int64_t current_time, 
                           const CombatUnit& source,
                           const CombatUnit* target,
                           const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                           const std::vector<std::shared_ptr<CombatUnit>>& enemies) const {
  // Check if the source is stunned or silenced
  if (source.is_stunned() || source.is_silenced()) {
    return false;
  }
  
  // Check cooldown with ability haste
  double haste = source.combat_details().ability_haste;
  int64_t effective_cooldown = cooldown_duration_;
  
  if (haste > 0) {
    effective_cooldown = static_cast<int64_t>(cooldown_duration_ * 100.0 / (100.0 + haste));
  }
  
  if (last_used_ + effective_cooldown > current_time) {
    return false;
  }
  
  // If no triggers, ability should always trigger when off cooldown
  if (triggers_.empty()) {
    return true;
  }
  
  // Check all triggers - all must be active for the ability to trigger
  for (const auto& trigger : triggers_) {
    if (!trigger->IsActive(source, target, friendlies, enemies, current_time)) {
      return false;
    }
  }
  
  return true;
}

}  // namespace combat_simulator
