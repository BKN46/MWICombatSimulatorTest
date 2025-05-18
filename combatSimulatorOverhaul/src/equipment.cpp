#include "equipment.h"
#include <fstream>
#include <stdexcept>

namespace combat_simulator {

// Load JSON data from files
const nlohmann::json Equipment::item_detail_map_ = []() {
  std::ifstream file("data/itemDetailMap.json");
  nlohmann::json data;
  file >> data;
  return data;
}();

const nlohmann::json Equipment::enhancement_level_total_multiplier_table_ = []() {
  std::ifstream file("data/enhancementLevelTotalBonusMultiplierTable.json");
  nlohmann::json data;
  file >> data;
  return data;
}();

Equipment::Equipment(const std::string& hrid, int enhancement_level)
    : hrid_(hrid), enhancement_level_(enhancement_level) {
  if (!item_detail_map_.contains(hrid)) {
    throw std::runtime_error("No equipment found for hrid: " + hrid);
  }
  game_item_ = item_detail_map_[hrid];
}

Equipment Equipment::create_from_dto(const nlohmann::json& dto) {
  return Equipment(dto["hrid"], dto["enhancementLevel"]);
}

double Equipment::get_combat_stat(const std::string& combat_stat) const {
  double multiplier = enhancement_level_total_multiplier_table_[std::to_string(enhancement_level_)];
  
  if (game_item_["equipmentDetail"]["combatStats"].contains(combat_stat)) {
    double enhancement_bonus = 0.0;
    if (game_item_["equipmentDetail"]["combatEnhancementBonuses"].contains(combat_stat)) {
      enhancement_bonus = game_item_["equipmentDetail"]["combatEnhancementBonuses"][combat_stat];
    }
    
    double stat = game_item_["equipmentDetail"]["combatStats"][combat_stat] + multiplier * enhancement_bonus;
    return stat;
  }
  
  return 0.0;
}

std::string Equipment::get_combat_style() const {
  return game_item_["equipmentDetail"]["combatStats"]["combatStyleHrids"][0];
}

std::string Equipment::get_damage_type() const {
  return game_item_["equipmentDetail"]["combatStats"]["damageType"];
}

} // namespace combat_simulator
