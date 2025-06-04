// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the Consumable class for the combat simulator.

#include "consumable.h"

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

// Static helper function to load item detail map
nlohmann::json Consumable::LoadItemDetailMap() {
  static nlohmann::json item_detail_map = nullptr;
  if (item_detail_map == nullptr) {
    try {
      std::ifstream file("data/itemDetailMap.json");
      if (file.is_open()) {
        item_detail_map = nlohmann::json::parse(file);
      } else {
        // Throw exception if file not found
        throw std::runtime_error("Failed to open itemDetailMap.json");
      }
    } catch (const std::exception& e) {
      // Rethrow with more context
      throw std::runtime_error(std::string("Error loading item details: ") + e.what());
    }
  }
  return item_detail_map;
}

Consumable::Consumable(const std::string& hrid, 
                     const std::vector<std::shared_ptr<Trigger>>& triggers) 
    : hrid_(hrid) {
  // Load consumable details from the item detail map
  auto item_detail_map = LoadItemDetailMap();
  
  if (item_detail_map.contains(hrid)) {
    const auto& game_consumable = item_detail_map[hrid];
    
    // Initialize basic properties from consumable detail
    cooldown_duration_ = game_consumable["consumableDetail"]["cooldownDuration"];
    hitpoint_restore_ = game_consumable["consumableDetail"]["hitpointRestore"];
    manapoint_restore_ = game_consumable["consumableDetail"]["manapointRestore"];
    recovery_duration_ = game_consumable["consumableDetail"]["recoveryDuration"];
    category_hrid_ = game_consumable["categoryHrid"];
    
    // Process buffs if they exist
    if (game_consumable["consumableDetail"].contains("buffs") && 
        !game_consumable["consumableDetail"]["buffs"].is_null()) {
      for (const auto& buff_json : game_consumable["consumableDetail"]["buffs"]) {
        Buff buff;
        if (buff_json.contains("uniqueHrid")) {
          buff.set_unique_hrid(buff_json["uniqueHrid"]);
        }
        if (buff_json.contains("typeHrid")) {
          buff.set_type_hrid(buff_json["typeHrid"]);
        }
        
        // Set buff properties
        buff.set_ratio_boost(buff_json.value("ratioBoost", 0.0));
        buff.set_flat_boost(buff_json.value("flatBoost", 0.0));
        
        if (buff_json.contains("duration")) {
          buff.set_duration(buff_json["duration"]);
        }
        
        buffs_.push_back(buff);
      }
    }
    
    // Set triggers
    if (!triggers.empty()) {
      triggers_ = triggers;
    } else if (game_consumable["consumableDetail"].contains("defaultCombatTriggers") && 
               !game_consumable["consumableDetail"]["defaultCombatTriggers"].is_null()) {
      // Create triggers from default combat triggers
      for (const auto& trigger_json : game_consumable["consumableDetail"]["defaultCombatTriggers"]) {
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
  } else {
    throw std::runtime_error("No consumable found for hrid: " + hrid);
  }
}

std::unique_ptr<Consumable> Consumable::CreateFromDTO(const nlohmann::json& dto) {
  std::vector<std::shared_ptr<Trigger>> triggers;
  
  // Create triggers from DTO
  for (const auto& trigger_json : dto["triggers"]) {
    triggers.push_back(Trigger::CreateFromDTO(trigger_json));
  }
  
  // Create and return a new Consumable
  return std::make_unique<Consumable>(
      dto["hrid"],
      triggers
  );
}

bool Consumable::ShouldTrigger(int64_t current_time, 
                             const CombatUnit& source,
                             const CombatUnit* target,
                             const std::vector<std::shared_ptr<CombatUnit>>& friendlies,
                             const std::vector<std::shared_ptr<CombatUnit>>& enemies) const {
  // Check if the source is stunned
  if (source.is_stunned()) {
    return false;
  }
  
  // Determine consumable haste based on category
  double consumable_haste = 0.0;
  if (category_hrid_.find("food") != std::string::npos) {
    consumable_haste = source.combat_details().combat_stats.food_haste;
  } else {
    consumable_haste = source.combat_details().combat_stats.drink_concentration;
  }
  
  // Calculate effective cooldown with haste
  int64_t effective_cooldown = cooldown_duration_;
  if (consumable_haste > 0) {
    effective_cooldown = static_cast<int64_t>(cooldown_duration_ / (1 + consumable_haste));
  }
  
  // Check if the consumable is still on cooldown
  if (last_used_ + effective_cooldown > current_time) {
    return false;
  }
  
  // If no triggers, consumable should always trigger when off cooldown
  if (triggers_.empty()) {
    return true;
  }
  
  // Check all triggers - all must be active for the consumable to trigger
  for (const auto& trigger : triggers_) {
    if (!trigger->IsActive(source, target, friendlies, enemies, current_time)) {
      return false;
    }
  }
  
  return true;
}

}  // namespace combat_simulator
