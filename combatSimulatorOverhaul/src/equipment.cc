// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the Equipment class for the combat simulator.

#include "equipment.h"

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

namespace combat_simulator {

// Static helper function to load item detail map
nlohmann::json Equipment::LoadItemDetailMap() {
  static nlohmann::json item_detail_map = nullptr;
  if (item_detail_map == nullptr) {
    try {
      std::ifstream file("combatSimulatorOverhaul/data/itemDetailMap.json");
      if (file.is_open()) {
        item_detail_map = nlohmann::json::parse(file);
      } else {
        // Fallback if file not found
        item_detail_map = nlohmann::json::object();
      }
    } catch (const std::exception& e) {
      // Handle parsing errors
      item_detail_map = nlohmann::json::object();
    }
  }
  return item_detail_map;
}

// Static helper function to load enhancement level total multiplier table
nlohmann::json Equipment::LoadEnhancementLevelTotalMultiplierTable() {
  static nlohmann::json enhancement_level_table = nullptr;
  if (enhancement_level_table == nullptr) {
    try {
      std::ifstream file("combatSimulatorOverhaul/data/enhancementLevelTotalBonusMultiplierTable.json");
      if (file.is_open()) {
        enhancement_level_table = nlohmann::json::parse(file);
      } else {
        // Fallback if file not found
        enhancement_level_table = nlohmann::json::object();
      }
    } catch (const std::exception& e) {
      // Handle parsing errors
      enhancement_level_table = nlohmann::json::object();
    }
  }
  return enhancement_level_table;
}

Equipment::Equipment(const std::string& hrid, int enhancement_level) 
    : hrid_(hrid), enhancement_level_(enhancement_level) {
  // Load item details from the detail map
  auto item_detail_map = LoadItemDetailMap();
  
  if (item_detail_map.contains(hrid)) {
    game_item_ = item_detail_map[hrid];
  } else {
    throw std::runtime_error("No equipment found for hrid: " + hrid);
  }
}

std::unique_ptr<Equipment> Equipment::CreateFromDTO(const nlohmann::json& dto) {
  return std::make_unique<Equipment>(
      dto["hrid"],
      dto["enhancementLevel"]
  );
}

double Equipment::GetCombatStat(const std::string& combat_stat) const {
  auto enhancement_level_table = LoadEnhancementLevelTotalMultiplierTable();
  double multiplier = enhancement_level_table[std::to_string(enhancement_level_)];
  if (game_item_["equipmentDetail"]["combatStats"].contains(combat_stat)) {
    double enhancement_bonus = 0.0;
    if (game_item_["equipmentDetail"]["combatEnhancementBonuses"].contains(combat_stat)) {
      enhancement_bonus = game_item_["equipmentDetail"]["combatEnhancementBonuses"][combat_stat];
    }
    double stat = game_item_["equipmentDetail"]["combatStats"][combat_stat].get<double>() +
                 multiplier * enhancement_bonus;
    return stat;
  }
  return 0.0;
}

std::string Equipment::GetCombatStyle() const {
  if (game_item_["equipmentDetail"]["combatStats"].contains("combatStyleHrids") && 
      game_item_["equipmentDetail"]["combatStats"]["combatStyleHrids"].is_array() &&
      !game_item_["equipmentDetail"]["combatStats"]["combatStyleHrids"].empty()) {
    return game_item_["equipmentDetail"]["combatStats"]["combatStyleHrids"][0];
  }
  
  return "";
}

std::string Equipment::GetDamageType() const {
  if (game_item_["equipmentDetail"]["combatStats"].contains("damageType")) {
    return game_item_["equipmentDetail"]["combatStats"]["damageType"];
  }
  
  return "";
}

}  // namespace combat_simulator
