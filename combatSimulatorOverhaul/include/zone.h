#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "monster.h"

namespace combat_simulator_overhaul {

class Zone {
 public:
  // Constructor that takes an HRID
  explicit Zone(const std::string& hrid);

  // Get a random encounter of monsters
  std::vector<std::shared_ptr<Monster>> GetRandomEncounter();
  
  // Increment dungeon failures count and reset encounters
  void FailWave();
  
  // Get the next wave of monsters in a dungeon
  std::vector<std::shared_ptr<Monster>> GetNextWave();
  
  // Getters
  const std::string& hrid() const { return hrid_; }
  bool is_dungeon() const { return is_dungeon_; }
  int dungeons_completed() const { return dungeons_completed_; }
  int dungeons_failed() const { return dungeons_failed_; }
  bool final_wave() const { return final_wave_; }
  
 private:
  std::string hrid_;
  std::unordered_map<std::string, std::any> monster_spawn_info_;
  std::unordered_map<std::string, std::any> dungeon_spawn_info_;
  int encounters_killed_ = 1;
  std::vector<std::unordered_map<std::string, std::any>> buffs_;
  bool is_dungeon_ = false;
  int dungeons_completed_ = 0;
  int dungeons_failed_ = 0;
  bool final_wave_ = false;
};

} // namespace combat_simulator_overhaul
