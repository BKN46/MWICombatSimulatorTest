#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace combat_simulator {

class Equipment {
 public:
  // Constructor with hrid and enhancement level
  Equipment(const std::string& hrid, int enhancement_level);

  // Factory method to create from DTO
  static Equipment create_from_dto(const nlohmann::json& dto);

  // Get combat stat value based on stat name
  double get_combat_stat(const std::string& combat_stat) const;

  // Get the combat style
  std::string get_combat_style() const;

  // Get damage type
  std::string get_damage_type() const;

 private:
  std::string hrid_;
  int enhancement_level_;
  nlohmann::json game_item_;

  static const nlohmann::json item_detail_map_;
  static const nlohmann::json enhancement_level_total_multiplier_table_;
};

} // namespace combat_simulator
