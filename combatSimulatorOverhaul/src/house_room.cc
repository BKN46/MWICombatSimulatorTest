// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the HouseRoom class for the combat simulator.

#include "house_room.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "buff.h"
#include "nlohmann/json.hpp"

namespace {
// Helper function to load the house room detail map
nlohmann::json LoadHouseRoomDetailMap() {
  static nlohmann::json house_room_detail_map = nlohmann::json::object();
  static bool initialized = false;
  if (!initialized) {
    try {
      std::ifstream file("data/houseRoomDetailMap.json");
      if (file.is_open()) {
        house_room_detail_map = nlohmann::json::parse(file);
        initialized = true;
      } else {
        // Fallback if file not found
        throw std::runtime_error("Failed to open houseRoomDetailMap.json");
      }
    } catch (const std::exception& e) {
      // Handle parsing errors
      throw std::runtime_error(std::string("Error loading house room details: ") + e.what());
    }
  }
  return house_room_detail_map;
}
}  // namespace

namespace combat_simulator {

HouseRoom::HouseRoom(const std::string& hrid, int level) 
    : hrid_(hrid), level_(level) {
  // Load house room details from the detail map
  nlohmann::json game_house_room;
  auto house_room_detail_map = LoadHouseRoomDetailMap();
  
  if (house_room_detail_map.contains(hrid)) {
    game_house_room = house_room_detail_map[hrid];
  } else {
    throw std::runtime_error("No house room found for hrid: " + hrid);
  }
  
  // Process action buffs if they exist
  if (game_house_room.contains("actionBuffs") && !game_house_room["actionBuffs"].is_null()) {
    for (const auto& action_buff_json : game_house_room["actionBuffs"]) {
      Buff buff;
      if (action_buff_json.contains("uniqueHrid")) {
        buff.set_unique_hrid(action_buff_json["uniqueHrid"]);
      }
      if (action_buff_json.contains("typeHrid")) {
        buff.set_type_hrid(action_buff_json["typeHrid"]);
      }
      
      // Apply level scaling
      double ratio_boost = action_buff_json.value("ratioBoost", 0.0);
      double ratio_boost_level_bonus = action_buff_json.value("ratioBoostLevelBonus", 0.0);
      buff.set_ratio_boost(ratio_boost + (level - 1) * ratio_boost_level_bonus);
      
      double flat_boost = action_buff_json.value("flatBoost", 0.0);
      double flat_boost_level_bonus = action_buff_json.value("flatBoostLevelBonus", 0.0);
      buff.set_flat_boost(flat_boost + (level - 1) * flat_boost_level_bonus);
      
      if (action_buff_json.contains("duration")) {
        buff.set_duration(action_buff_json["duration"]);
      }
      
      buffs_.push_back(buff);
    }
  }
  
  // Process global buffs if they exist
  if (game_house_room.contains("globalBuffs") && !game_house_room["globalBuffs"].is_null()) {
    for (const auto& global_buff_json : game_house_room["globalBuffs"]) {
      Buff buff;
      if (global_buff_json.contains("uniqueHrid")) {
        buff.set_unique_hrid(global_buff_json["uniqueHrid"]);
      }
      if (global_buff_json.contains("typeHrid")) {
        buff.set_type_hrid(global_buff_json["typeHrid"]);
      }
      
      // Apply level scaling
      double ratio_boost = global_buff_json.value("ratioBoost", 0.0);
      double ratio_boost_level_bonus = global_buff_json.value("ratioBoostLevelBonus", 0.0);
      buff.set_ratio_boost(ratio_boost + (level - 1) * ratio_boost_level_bonus);
      
      double flat_boost = global_buff_json.value("flatBoost", 0.0);
      double flat_boost_level_bonus = global_buff_json.value("flatBoostLevelBonus", 0.0);
      buff.set_flat_boost(flat_boost + (level - 1) * flat_boost_level_bonus);
      
      if (global_buff_json.contains("duration")) {
        buff.set_duration(global_buff_json["duration"]);
      }
      
      buffs_.push_back(buff);
    }
  }
}

}  // namespace combat_simulator
