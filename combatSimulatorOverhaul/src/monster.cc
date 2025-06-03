// Copyright 2025 BKNMWICombatSimulator
//
// monster.cc - Implementation of the Monster class which represents a combat monster
// entity in the simulation system.

#include "monster.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "ability.h"
#include "drops.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Helper function to load the combat monster detail map from JSON
nlohmann::json Monster::LoadCombatMonsterDetailMap() {
  static nlohmann::json combat_monster_detail_map = nullptr;
  if (combat_monster_detail_map == nullptr) {
    try {
      std::ifstream file("combatSimulatorOverhaul/data/combatMonsterDetailMap.json");
      if (!file.is_open()) {
        throw std::runtime_error("Failed to open combatMonsterDetailMap.json");
      }
      file >> combat_monster_detail_map;
    } catch (const std::exception& e) {
      std::cerr << "Error loading combat monster detail map: " << e.what() << std::endl;
      combat_monster_detail_map = nlohmann::json::object();
    }
  }
  return combat_monster_detail_map;
}

// Constructor with HRID and elite tier
Monster::Monster(const std::string& hrid, int elite_tier) : elite_tier_(elite_tier) {
  is_player_ = false;
  hrid_ = hrid;
  
  // Load monster details from JSON
  nlohmann::json monster_detail_map = LoadCombatMonsterDetailMap();
  
  // Check if monster exists in the detail map
  if (monster_detail_map.find(hrid) == monster_detail_map.end()) {
    throw std::runtime_error("No monster found for hrid: " + hrid);
  }
  
  // Get monster data
  const nlohmann::json& game_monster = monster_detail_map[hrid];
  
  // Load abilities
  if (game_monster.contains("abilities") && game_monster["abilities"].is_array()) {
    for (size_t i = 0; i < game_monster["abilities"].size(); ++i) {
      const auto& ability_data = game_monster["abilities"][i];
      
      // Skip abilities that require a higher elite tier
      if (ability_data.contains("minEliteTier") && 
          ability_data["minEliteTier"].get<int>() > elite_tier_) {
        continue;
      }
      
      // Add ability to the monster
      if (ability_data.contains("abilityHrid") && ability_data.contains("level")) {
        std::string ability_hrid = ability_data["abilityHrid"].get<std::string>();
        int level = ability_data["level"].get<int>();
        
        abilities_.push_back(std::make_shared<Ability>(ability_hrid, level));
      }
    }
  }
  
  // Load drop table
  if (game_monster.contains("dropTable") && game_monster["dropTable"].is_array()) {
    for (size_t i = 0; i < game_monster["dropTable"].size(); ++i) {
      const auto& drop_data = game_monster["dropTable"][i];
      
      if (drop_data.contains("itemHrid") && 
          drop_data.contains("dropRate") && 
          drop_data.contains("minCount") && 
          drop_data.contains("maxCount")) {
        
        std::string item_hrid = drop_data["itemHrid"].get<std::string>();
        double drop_rate = drop_data["dropRate"].get<double>();
        int min_count = drop_data["minCount"].get<int>();
        int max_count = drop_data["maxCount"].get<int>();
        
        // Use elite tier from the drop data if available, otherwise use monster's elite tier
        int drop_elite_tier = drop_data.contains("eliteTier") ? 
                              drop_data["eliteTier"].get<int>() : 
                              drop_data.contains("minEliteTier") ? 
                              drop_data["minEliteTier"].get<int>() : 0;
        
        drop_table_.push_back(nlohmann::json{
          {"itemHrid", item_hrid},
          {"dropRate", drop_rate},
          {"minCount", min_count},
          {"maxCount", max_count},
          {"eliteTier", drop_elite_tier}
        });
      }
    }
  }
  
  // Load rare drop table
  if (game_monster.contains("rareDropTable") && game_monster["rareDropTable"].is_array()) {
    for (size_t i = 0; i < game_monster["rareDropTable"].size(); ++i) {
      const auto& drop_data = game_monster["rareDropTable"][i];
      
      if (drop_data.contains("itemHrid") && 
          drop_data.contains("dropRate") && 
          drop_data.contains("minCount")) {
        
        std::string item_hrid = drop_data["itemHrid"].get<std::string>();
        double drop_rate = drop_data["dropRate"].get<double>();
        int min_count = drop_data["minCount"].get<int>();
        
        // For max count, use the provided value or default to min_count
        int max_count = drop_data.contains("maxCount") ? 
                        drop_data["maxCount"].get<int>() : min_count;
        
        // Use elite tier from drop data or default values
        int drop_elite_tier;
        if (drop_data.contains("eliteTier")) {
          drop_elite_tier = drop_data["eliteTier"].get<int>();
        } else if (drop_data.contains("minEliteTier")) {
          drop_elite_tier = drop_data["minEliteTier"].get<int>();
        } else if (i < drop_table_.size() && drop_table_[i].contains("eliteTier")) {
          drop_elite_tier = drop_table_[i]["eliteTier"].get<int>();
        } else {
          drop_elite_tier = 0;
        }
        
        rare_drop_table_.push_back(nlohmann::json{
          {"itemHrid", item_hrid},
          {"dropRate", drop_rate},
          {"minCount", min_count},
          {"maxCount", max_count},
          {"eliteTier", drop_elite_tier}
        });
      }
    }
  }
  
  // Initialize and update combat details
  UpdateCombatDetails();
}

// Updates combat details based on elite tier
void Monster::UpdateCombatDetails() {
  nlohmann::json monster_detail_map = LoadCombatMonsterDetailMap();
  const nlohmann::json& game_monster = monster_detail_map[hrid_];
  
  // Select combat details based on elite tier
  nlohmann::json combat_details;
  std::string combat_details_key;
  
  switch (elite_tier_) {
    case 2:
      combat_details_key = "elite2CombatDetails";
      break;
    case 1:
      combat_details_key = "elite1CombatDetails";
      break;
    default:
      combat_details_key = "combatDetails";
      break;
  }
  
  // If elite tier specific details don't exist, fall back to base combat details
  if (game_monster.contains(combat_details_key)) {
    combat_details = game_monster[combat_details_key];
  } else {
    combat_details = game_monster["combatDetails"];
  }
  
  // Update base levels
  if (combat_details.contains("staminaLevel")) {
    stamina_level_ = combat_details["staminaLevel"].get<int>();
  }
  if (combat_details.contains("intelligenceLevel")) {
    intelligence_level_ = combat_details["intelligenceLevel"].get<int>();
  }
  if (combat_details.contains("attackLevel")) {
    attack_level_ = combat_details["attackLevel"].get<int>();
  }
  if (combat_details.contains("powerLevel")) {
    power_level_ = combat_details["powerLevel"].get<int>();
  }
  if (combat_details.contains("defenseLevel")) {
    defense_level_ = combat_details["defenseLevel"].get<int>();
  }
  if (combat_details.contains("rangedLevel")) {
    ranged_level_ = combat_details["rangedLevel"].get<int>();
  }
  if (combat_details.contains("magicLevel")) {
    magic_level_ = combat_details["magicLevel"].get<int>();
  }
  
  // Update combat style from the first available combat style in the array
  if (combat_details.contains("combatStats") && 
      combat_details["combatStats"].contains("combatStyleHrids") && 
      combat_details["combatStats"]["combatStyleHrids"].is_array() && 
      !combat_details["combatStats"]["combatStyleHrids"].empty()) {
    
    combat_details_.combat_stats.combat_style_hrid = 
        combat_details["combatStats"]["combatStyleHrids"][0].get<std::string>();
  }
  
  // Update all combat stats
  if (combat_details.contains("combatStats")) {
    const auto& combat_stats = combat_details["combatStats"];
    
    for (auto it = combat_stats.begin(); it != combat_stats.end(); ++it) {
      std::string key = it.key();
      
      // Convert JavaScript camelCase to C++ snake_case for setting stats
      if (key == "stabAccuracy") {
        combat_details_.combat_stats.stab_accuracy = it.value().get<double>();
      } else if (key == "slashAccuracy") {
        combat_details_.combat_stats.slash_accuracy = it.value().get<double>();
      } else if (key == "smashAccuracy") {
        combat_details_.combat_stats.smash_accuracy = it.value().get<double>();
      } else if (key == "rangedAccuracy") {
        combat_details_.combat_stats.ranged_accuracy = it.value().get<double>();
      } else if (key == "magicAccuracy") {
        combat_details_.combat_stats.magic_accuracy = it.value().get<double>();
      } else if (key == "stabDamage") {
        combat_details_.combat_stats.stab_damage = it.value().get<double>();
      } else if (key == "slashDamage") {
        combat_details_.combat_stats.slash_damage = it.value().get<double>();
      } else if (key == "smashDamage") {
        combat_details_.combat_stats.smash_damage = it.value().get<double>();
      } else if (key == "rangedDamage") {
        combat_details_.combat_stats.ranged_damage = it.value().get<double>();
      } else if (key == "magicDamage") {
        combat_details_.combat_stats.magic_damage = it.value().get<double>();
      } else if (key == "taskDamage") {
        combat_details_.combat_stats.task_damage = it.value().get<double>();
      } else if (key == "physicalAmplify") {
        combat_details_.combat_stats.physical_amplify = it.value().get<double>();
      } else if (key == "waterAmplify") {
        combat_details_.combat_stats.water_amplify = it.value().get<double>();
      } else if (key == "natureAmplify") {
        combat_details_.combat_stats.nature_amplify = it.value().get<double>();
      } else if (key == "fireAmplify") {
        combat_details_.combat_stats.fire_amplify = it.value().get<double>();
      } else if (key == "healingAmplify") {
        combat_details_.combat_stats.healing_amplify = it.value().get<double>();
      } else if (key == "stabEvasion") {
        combat_details_.combat_stats.stab_evasion = it.value().get<double>();
      } else if (key == "slashEvasion") {
        combat_details_.combat_stats.slash_evasion = it.value().get<double>();
      } else if (key == "smashEvasion") {
        combat_details_.combat_stats.smash_evasion = it.value().get<double>();
      } else if (key == "rangedEvasion") {
        combat_details_.combat_stats.ranged_evasion = it.value().get<double>();
      } else if (key == "magicEvasion") {
        combat_details_.combat_stats.magic_evasion = it.value().get<double>();
      } else if (key == "armor") {
        combat_details_.combat_stats.armor = it.value().get<double>();
      } else if (key == "waterResistance") {
        combat_details_.combat_stats.water_resistance = it.value().get<double>();
      } else if (key == "natureResistance") {
        combat_details_.combat_stats.nature_resistance = it.value().get<double>();
      } else if (key == "fireResistance") {
        combat_details_.combat_stats.fire_resistance = it.value().get<double>();
      } else if (key == "maxHitpoints") {
        combat_details_.combat_stats.max_hitpoints = it.value().get<double>();
      } else if (key == "maxManapoints") {
        combat_details_.combat_stats.max_manapoints = it.value().get<double>();
      } else if (key == "lifeSteal") {
        combat_details_.combat_stats.life_steal = it.value().get<double>();
      } else if (key == "hpRegenPer10") {
        combat_details_.combat_stats.hp_regen_per_10 = it.value().get<double>();
      } else if (key == "mpRegenPer10") {
        combat_details_.combat_stats.mp_regen_per_10 = it.value().get<double>();
      } else if (key == "physicalThorns") {
        combat_details_.combat_stats.physical_thorns = it.value().get<double>();
      } else if (key == "elementalThorns") {
        combat_details_.combat_stats.elemental_thorns = it.value().get<double>();
      } else if (key == "combatDropRate") {
        combat_details_.combat_stats.combat_drop_rate = it.value().get<double>();
      } else if (key == "combatRareFind") {
        combat_details_.combat_stats.combat_rare_find = it.value().get<double>();
      } else if (key == "combatDropQuantity") {
        combat_details_.combat_stats.combat_drop_quantity = it.value().get<double>();
      } else if (key == "combatExperience") {
        combat_details_.combat_stats.combat_experience = it.value().get<double>();
      } else if (key == "criticalRate") {
        combat_details_.combat_stats.critical_rate = it.value().get<double>();
      } else if (key == "criticalDamage") {
        combat_details_.combat_stats.critical_damage = it.value().get<double>();
      } else if (key == "armorPenetration") {
        combat_details_.combat_stats.armor_penetration = it.value().get<double>();
      } else if (key == "waterPenetration") {
        combat_details_.combat_stats.water_penetration = it.value().get<double>();
      } else if (key == "naturePenetration") {
        combat_details_.combat_stats.nature_penetration = it.value().get<double>();
      } else if (key == "firePenetration") {
        combat_details_.combat_stats.fire_penetration = it.value().get<double>();
      } else if (key == "abilityHaste") {
        combat_details_.ability_haste = it.value().get<double>();
      } else if (key == "tenacity") {
        combat_details_.tenacity = it.value().get<double>();
      } else if (key == "manaLeech") {
        combat_details_.combat_stats.mana_leech = it.value().get<double>();
      } else if (key == "castSpeed") {
        combat_details_.combat_stats.cast_speed = it.value().get<double>();
      } else if (key == "threat") {
        combat_details_.combat_stats.threat = it.value().get<double>();
        combat_details_.total_threat = it.value().get<double>();
      } else if (key == "parry") {
        combat_details_.combat_stats.parry = it.value().get<double>();
      } else if (key == "mayhem") {
        combat_details_.combat_stats.mayhem = it.value().get<double>();
      } else if (key == "pierce") {
        combat_details_.combat_stats.pierce = it.value().get<double>();
      } else if (key == "curse") {
        combat_details_.combat_stats.curse = it.value().get<double>();
      } else if (key == "fury") {
        combat_details_.combat_stats.fury = it.value().get<double>();
      } else if (key == "weaken") {
        combat_details_.combat_stats.weaken = it.value().get<double>();
      } else if (key == "ripple") {
        combat_details_.combat_stats.ripple = it.value().get<double>();
      } else if (key == "bloom") {
        combat_details_.combat_stats.bloom = it.value().get<double>();
      } else if (key == "blaze") {
        combat_details_.combat_stats.blaze = it.value().get<double>();
      } else if (key == "attackSpeed") {
        combat_details_.combat_stats.attack_speed = it.value().get<double>();
      } else if (key == "foodHaste") {
        combat_details_.combat_stats.food_haste = it.value().get<double>();
      } else if (key == "drinkConcentration") {
        combat_details_.combat_stats.drink_concentration = it.value().get<double>();
      } else if (key == "autoAttackDamage") {
        combat_details_.combat_stats.auto_attack_damage = it.value().get<double>();
      }
    }
  }
  
  // Set attack interval
  if (combat_details.contains("attackInterval")) {
    combat_details_.combat_stats.attack_interval = combat_details["attackInterval"].get<int64_t>();
  }
  
  // Initialize any missing stats to zero
  const std::vector<std::string> stat_keys = {
    "stab_accuracy", "slash_accuracy", "smash_accuracy", "ranged_accuracy", "magic_accuracy",
    "stab_damage", "slash_damage", "smash_damage", "ranged_damage", "magic_damage",
    "task_damage", "physical_amplify", "water_amplify", "nature_amplify", "fire_amplify",
    "healing_amplify", "stab_evasion", "slash_evasion", "smash_evasion", "ranged_evasion",
    "magic_evasion", "armor", "water_resistance", "nature_resistance", "fire_resistance",
    "max_hitpoints", "max_manapoints", "life_steal", "hp_regen_per_10", "mp_regen_per_10",
    "physical_thorns", "elemental_thorns", "combat_drop_rate", "combat_rare_find", 
    "combat_drop_quantity", "combat_experience", "critical_rate", "critical_damage",
    "armor_penetration", "water_penetration", "nature_penetration", "fire_penetration",
    "mana_leech", "cast_speed", "threat", "parry", "mayhem", "pierce", "curse", "fury",
    "weaken", "ripple", "bloom", "blaze", "attack_speed", "food_haste", "drink_concentration",
    "auto_attack_damage"
  };
  
  for (const auto& key : stat_keys) {
    double value = combat_details_.combat_stats.GetStat(key);
    if (value == 0.0) {
      combat_details_.combat_stats.SetStat(key, 0.0);
    }
  }
  
  // Call the parent class's update method to handle common CombatUnit updates
  CombatUnit::UpdateCombatDetails();
}

}  // namespace combat_simulator
