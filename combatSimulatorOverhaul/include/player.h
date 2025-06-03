// Copyright 2025 BKNMWICombatSimulator
//
// player.h - Definition of the Player class which represents a player entity in the simulation system.

#ifndef COMBAT_SIMULATOR_PLAYER_H_
#define COMBAT_SIMULATOR_PLAYER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ability.h"
#include "combat_unit.h"
#include "consumable.h"
#include "equipment.h"
#include "house_room.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

class Player : public CombatUnit {
 public:
  Player();
  ~Player() override = default;

  // Factory method to create a Player from a DTO (JSON object)
  static std::unique_ptr<Player> CreateFromDTO(const nlohmann::json& dto);

  // Updates combat details based on equipment and stats
  void UpdateCombatDetails() override;

  // Equipment accessors
  std::map<std::string, std::shared_ptr<Equipment>>& equipment() { return equipment_; }
  const std::map<std::string, std::shared_ptr<Equipment>>& equipment() const { return equipment_; }

  // Food, drinks, abilities, house rooms accessors
  std::vector<std::shared_ptr<Consumable>>& food() { return food_; }
  std::vector<std::shared_ptr<Consumable>>& drinks() { return drinks_; }
  std::vector<std::shared_ptr<Ability>>& abilities() { return abilities_; }
  std::vector<std::shared_ptr<HouseRoom>>& house_rooms() { return house_rooms_; }

  // Player-specific fields
  bool is_player() const { return is_player_; }

 private:
  std::map<std::string, std::shared_ptr<Equipment>> equipment_;
  std::vector<std::shared_ptr<Consumable>> food_;
  std::vector<std::shared_ptr<Consumable>> drinks_;
  std::vector<std::shared_ptr<Ability>> abilities_;
  std::vector<std::shared_ptr<HouseRoom>> house_rooms_;
  bool is_player_ = true;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_PLAYER_H_
