#include "house_room.h"

#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

namespace combat_simulator {

// Using nlohmann/json for JSON parsing
using json = nlohmann::json;

namespace {
// Load and cache the house room detail map
json LoadHouseRoomDetailMap() {
  static json house_room_detail_map = nullptr;
  if (house_room_detail_map == nullptr) {
    std::ifstream file("data/houseRoomDetailMap.json");
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open houseRoomDetailMap.json");
    }
    file >> house_room_detail_map;
  }
  return house_room_detail_map;
}
}  // namespace

HouseRoom::HouseRoom(const std::string& hrid, int level) 
    : hrid_(hrid), level_(level) {
  
  json house_room_detail_map = LoadHouseRoomDetailMap();
  if (!house_room_detail_map.contains(hrid_)) {
    throw std::runtime_error("No house room found for hrid: " + hrid_);
  }

  const json& game_house_room = house_room_detail_map[hrid_];

  // Process action buffs
  if (game_house_room.contains("actionBuffs") && game_house_room["actionBuffs"].is_array()) {
    for (const auto& action_buff : game_house_room["actionBuffs"]) {
      buffs_.emplace_back(action_buff, level_);
    }
  }

  // Process global buffs
  if (game_house_room.contains("globalBuffs") && game_house_room["globalBuffs"].is_array()) {
    for (const auto& global_buff : game_house_room["globalBuffs"]) {
      buffs_.emplace_back(global_buff, level_);
    }
  }
}

}  // namespace combat_simulator
