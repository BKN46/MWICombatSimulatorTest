// Copyright 2025 BKNMWICombatSimulator
//
// zone.h - Definition of the Zone class which represents a combat zone
// in the simulation system.

#ifndef COMBAT_SIMULATOR_ZONE_H_
#define COMBAT_SIMULATOR_ZONE_H_

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "monster.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Represents a combat zone in the game world
class Zone {
 public:
  // Default constructor
  Zone() = default;
  
  // Constructs a Zone with the specified HRID
  explicit Zone(const std::string& hrid);
  
  // Gets a random encounter of monsters based on zone rules
  std::vector<std::shared_ptr<Monster>> GetRandomEncounter();
  
  // Gets the next wave of monsters in a dungeon
  std::vector<std::shared_ptr<Monster>> GetNextWave();
  
  // Records a failure for the current wave in a dungeon
  void FailWave();
  
  // Getters
  const std::string& hrid() const { return hrid_; }
  bool is_dungeon() const { return is_dungeon_; }
  int encounters_killed() const { return encounters_killed_; }
  int dungeons_completed() const { return dungeons_completed_; }
  int dungeons_failed() const { return dungeons_failed_; }
  bool final_wave() const { return final_wave_; }
  const nlohmann::json& buffs() const { return buffs_; }
  
  // Setters
  void set_hrid(const std::string& hrid) { hrid_ = hrid; }
  void set_encounters_killed(int encounters_killed) { encounters_killed_ = encounters_killed; }
  
 private:
  // Helper function to load action detail map from JSON
  static nlohmann::json LoadActionDetailMap();
  
  // Random number generator
  std::mt19937 random_generator_{std::random_device{}()};
  
  // Member variables
  std::string hrid_;
  nlohmann::json monster_spawn_info_;
  nlohmann::json dungeon_spawn_info_;
  nlohmann::json buffs_;
  int encounters_killed_ = 1;
  bool is_dungeon_ = false;
  int dungeons_completed_ = 0;
  int dungeons_failed_ = 0;
  bool final_wave_ = false;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_ZONE_H_
