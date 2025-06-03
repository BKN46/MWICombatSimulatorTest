// Copyright 2025 BKNMWICombatSimulator
//
// equipment.h - Definition of the Equipment class which represents
// equipment items that can be used by combat units.

#ifndef COMBAT_SIMULATOR_EQUIPMENT_H_
#define COMBAT_SIMULATOR_EQUIPMENT_H_

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

namespace combat_simulator {

// Represents equipment that can be used by combat units
class Equipment {
 public:
  // Default constructor
  Equipment() = default;
  
  // Constructs an equipment item with the specified HRID and enhancement level
  Equipment(const std::string& hrid, int enhancement_level);
  
  // Creates an equipment from a data transfer object (JSON)
  static std::unique_ptr<Equipment> CreateFromDTO(const nlohmann::json& dto);
  
  // Gets a combat stat value for this equipment
  double GetCombatStat(const std::string& combat_stat) const;
  
  // Gets the combat style HRID for this equipment
  std::string GetCombatStyle() const;
  
  // Gets the damage type for this equipment
  std::string GetDamageType() const;
  
  // Getters
  const std::string& hrid() const { return hrid_; }
  int enhancement_level() const { return enhancement_level_; }
  const nlohmann::json& game_item() const { return game_item_; }
  
 private:
  // Helper function to load item detail map
  static nlohmann::json LoadItemDetailMap();
  
  // Helper function to load enhancement level total multiplier table
  static nlohmann::json LoadEnhancementLevelTotalMultiplierTable();
  
  // Member variables
  std::string hrid_;
  int enhancement_level_ = 0;
  nlohmann::json game_item_;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_EQUIPMENT_H_
