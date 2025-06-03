// Copyright 2025 BKNMWICombatSimulator
//
// house_room.h - Definition of the HouseRoom class that represents a room
// in a player's house, providing various buffs.

#ifndef COMBAT_SIMULATOR_HOUSE_ROOM_H_
#define COMBAT_SIMULATOR_HOUSE_ROOM_H_

#include <string>
#include <vector>

#include "buff.h"

namespace combat_simulator {

// Represents a room in a player's house.
// House rooms provide permanent buffs to the player.
class HouseRoom {
 public:
  // Constructs a house room with the specified HRID and level
  HouseRoom(const std::string& hrid, int level);
  
  // Default destructor
  ~HouseRoom() = default;
  
  // Returns the buffs provided by this house room
  const std::vector<Buff>& GetBuffs() const { return buffs_; }
  
  // Returns the HRID of this house room
  const std::string& hrid() const { return hrid_; }
  
  // Returns the level of this house room
  int level() const { return level_; }
  
 private:
  std::string hrid_;    // Human-readable ID for this house room
  int level_ = 1;       // Level of this house room
  std::vector<Buff> buffs_; // Buffs provided by this house room
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_HOUSE_ROOM_H_
