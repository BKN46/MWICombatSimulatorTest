// Copyright 2025 BKNMWICombatSimulator
//
// zone.cc - Implementation of the Zone class which represents a combat zone
// in the simulation system.

#include "zone.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "monster.h"
#include "nlohmann/json.hpp"

namespace combat_simulator {

// Helper function to load the action detail map from JSON
nlohmann::json Zone::LoadActionDetailMap() {
  static nlohmann::json action_detail_map = nullptr;
  if (action_detail_map == nullptr) {
    try {
      std::ifstream file("combatSimulatorOverhaul/data/actionDetailMap.json");
      if (!file.is_open()) {
        throw std::runtime_error("Failed to open actionDetailMap.json");
      }
      file >> action_detail_map;
    } catch (const std::exception& e) {
      std::cerr << "Error loading action detail map: " << e.what() << std::endl;
      action_detail_map = nlohmann::json::object();
    }
  }
  return action_detail_map;
}

// Constructor with HRID
Zone::Zone(const std::string& hrid) : hrid_(hrid) {
  // Load zone details from JSON
  nlohmann::json action_detail_map = LoadActionDetailMap();
  
  // Check if zone exists in the detail map
  if (action_detail_map.find(hrid) == action_detail_map.end()) {
    throw std::runtime_error("No zone found for hrid: " + hrid);
  }
  
  // Get zone data
  const nlohmann::json& game_zone = action_detail_map[hrid];
  
  // Check if the combatZoneInfo exists
  if (!game_zone.contains("combatZoneInfo") || game_zone["combatZoneInfo"].is_null()) {
    throw std::runtime_error("No combat zone info found for hrid: " + hrid);
  }
  
  // Get combat zone info
  const nlohmann::json& combat_zone_info = game_zone["combatZoneInfo"];
  
  // Load fight info and dungeon info
  if (combat_zone_info.contains("fightInfo") && !combat_zone_info["fightInfo"].is_null()) {
    monster_spawn_info_ = combat_zone_info["fightInfo"];
    
    // Set default battles per boss if not specified
    if (monster_spawn_info_.contains("battlesPerBoss")) {
      monster_spawn_info_["battlesPerBoss"] = 10;
    }
  }
  
  if (combat_zone_info.contains("dungeonInfo") && !combat_zone_info["dungeonInfo"].is_null()) {
    dungeon_spawn_info_ = combat_zone_info["dungeonInfo"];
  }
  
  // Load buffs
  if (game_zone.contains("buffs") && !game_zone["buffs"].is_null()) {
    buffs_ = game_zone["buffs"];
  }
  
  // Set is_dungeon flag
  if (combat_zone_info.contains("isDungeon") && !combat_zone_info["isDungeon"].is_null()) {
    is_dungeon_ = combat_zone_info["isDungeon"].get<bool>();
  }
  
  // Initialize encounters killed to 1
  encounters_killed_ = 1;
}

// Gets a random encounter of monsters
std::vector<std::shared_ptr<Monster>> Zone::GetRandomEncounter() {
  // Check if this is a boss encounter
  if (monster_spawn_info_.contains("bossSpawns") && 
      !monster_spawn_info_["bossSpawns"].is_null() && 
      monster_spawn_info_.contains("battlesPerBoss") && 
      encounters_killed_ == monster_spawn_info_["battlesPerBoss"].get<int>()) {
    
    // Reset encounters killed
    encounters_killed_ = 1;
    
    // Create boss monsters
    std::vector<std::shared_ptr<Monster>> boss_monsters;
    for (const auto& monster_data : monster_spawn_info_["bossSpawns"]) {
      if (monster_data.contains("combatMonsterHrid") && monster_data.contains("eliteTier")) {
        std::string monster_hrid = monster_data["combatMonsterHrid"].get<std::string>();
        int elite_tier = monster_data["eliteTier"].get<int>();
        boss_monsters.push_back(std::make_shared<Monster>(monster_hrid, elite_tier));
      }
    }
    
    return boss_monsters;
  }
  
  // Handle regular random encounters
  if (!monster_spawn_info_.contains("randomSpawnInfo") || 
      monster_spawn_info_["randomSpawnInfo"].is_null()) {
    return {};
  }
  
  const auto& random_spawn_info = monster_spawn_info_["randomSpawnInfo"];
  
  // Calculate total weight for random selection
  double total_weight = 0.0;
  for (const auto& spawn : random_spawn_info["spawns"]) {
    if (spawn.contains("rate")) {
      total_weight += spawn["rate"].get<double>();
    }
  }
  
  // Generate random monster encounter
  std::vector<nlohmann::json> encounter_hrids;
  double total_strength = 0.0;
  
  int max_spawn_count = random_spawn_info["maxSpawnCount"].get<int>();
  double max_total_strength = random_spawn_info["maxTotalStrength"].get<double>();
  
  for (int i = 0; i < max_spawn_count; ++i) {
    // Generate random weight
    std::uniform_real_distribution<double> dist(0.0, total_weight);
    double random_weight = dist(random_generator_);
    
    // Find monster based on random weight
    double cumulative_weight = 0.0;
    for (const auto& spawn : random_spawn_info["spawns"]) {
      if (spawn.contains("rate")) {
        cumulative_weight += spawn["rate"].get<double>();
        
        if (random_weight <= cumulative_weight) {
          // Check if adding this monster would exceed max strength
          if (spawn.contains("strength")) {
            double strength = spawn["strength"].get<double>();
            
            if (total_strength + strength <= max_total_strength) {
              // Add monster to encounter
              nlohmann::json monster_info;
              monster_info["hrid"] = spawn["combatMonsterHrid"].get<std::string>();
              monster_info["eliteTier"] = spawn["eliteTier"].get<int>();
              
              encounter_hrids.push_back(monster_info);
              total_strength += strength;
            } else {
              // Exceeds max strength, stop adding monsters
              break;
            }
          }
          
          // Found a monster, stop searching
          break;
        }
      }
    }
  }
  
  // Increment encounters killed
  encounters_killed_++;
  
  // Create Monster objects from HRIDs
  std::vector<std::shared_ptr<Monster>> monsters;
  for (const auto& monster_info : encounter_hrids) {
    monsters.push_back(std::make_shared<Monster>(
        monster_info["hrid"].get<std::string>(),
        monster_info["eliteTier"].get<int>()));
  }
  
  return monsters;
}

// Records a failure for the current wave in a dungeon
void Zone::FailWave() {
  dungeons_failed_++;
  encounters_killed_ = 1;
}

// Gets the next wave of monsters in a dungeon
std::vector<std::shared_ptr<Monster>> Zone::GetNextWave() {
  // Check if we've reached the maximum number of waves
  if (dungeon_spawn_info_.contains("maxWaves") &&
      encounters_killed_ > dungeon_spawn_info_["maxWaves"].get<int>()) {
    // Completed the dungeon
    dungeons_completed_++;
    encounters_killed_ = 1;
  }
  
  // Check if there's a fixed spawn for this wave
  if (dungeon_spawn_info_.contains("fixedSpawnsMap") && 
      !dungeon_spawn_info_["fixedSpawnsMap"].is_null() &&
      dungeon_spawn_info_["fixedSpawnsMap"].contains(std::to_string(encounters_killed_))) {
    
    // Get the fixed spawn monsters
    const auto& current_monsters = dungeon_spawn_info_["fixedSpawnsMap"][std::to_string(encounters_killed_)];
    encounters_killed_++;
    
    // Create Monster objects
    std::vector<std::shared_ptr<Monster>> monsters;
    for (const auto& monster_data : current_monsters) {
      if (monster_data.contains("combatMonsterHrid") && monster_data.contains("eliteTier")) {
        std::string monster_hrid = monster_data["combatMonsterHrid"].get<std::string>();
        int elite_tier = monster_data["eliteTier"].get<int>();
        monsters.push_back(std::make_shared<Monster>(monster_hrid, elite_tier));
      }
    }
    
    return monsters;
  }
  
  // Handle random spawns
  if (dungeon_spawn_info_.contains("randomSpawnInfoMap") &&
      !dungeon_spawn_info_["randomSpawnInfoMap"].is_null()) {
    
    // Find the appropriate spawn info for the current wave
    nlohmann::json monster_spawns;
    const auto& random_spawn_info_map = dungeon_spawn_info_["randomSpawnInfoMap"];
    
    // Get the wave keys and sort them
    std::vector<int> wave_keys;
    for (const auto& key_value : random_spawn_info_map.items()) {
      try {
        wave_keys.push_back(std::stoi(key_value.key()));
      } catch (const std::exception& e) {
        // Skip non-integer keys
      }
    }
    std::sort(wave_keys.begin(), wave_keys.end());
    
    // Find the appropriate spawn info based on current wave
    if (encounters_killed_ > wave_keys.back()) {
      // Use the highest wave key
      monster_spawns = random_spawn_info_map[std::to_string(wave_keys.back())];
    } else {
      // Find the appropriate range
      for (size_t i = 0; i < wave_keys.size() - 1; ++i) {
        if (encounters_killed_ >= wave_keys[i] && encounters_killed_ <= wave_keys[i+1]) {
          monster_spawns = random_spawn_info_map[std::to_string(wave_keys[i])];
          break;
        }
      }
    }
    
    // Calculate total weight for random selection
    double total_weight = 0.0;
    for (const auto& spawn : monster_spawns["spawns"]) {
      if (spawn.contains("rate")) {
        total_weight += spawn["rate"].get<double>();
      }
    }
    
    // Generate random monster encounter
    std::vector<nlohmann::json> encounter_hrids;
    double total_strength = 0.0;
    
    int max_spawn_count = monster_spawns["maxSpawnCount"].get<int>();
    double max_total_strength = monster_spawns["maxTotalStrength"].get<double>();
    
    for (int i = 0; i < max_spawn_count; ++i) {
      // Generate random weight
      std::uniform_real_distribution<double> dist(0.0, total_weight);
      double random_weight = dist(random_generator_);
      
      // Find monster based on random weight
      double cumulative_weight = 0.0;
      for (const auto& spawn : monster_spawns["spawns"]) {
        if (spawn.contains("rate")) {
          cumulative_weight += spawn["rate"].get<double>();
          
          if (random_weight <= cumulative_weight) {
            // Check if adding this monster would exceed max strength
            if (spawn.contains("strength")) {
              double strength = spawn["strength"].get<double>();
              
              if (total_strength + strength <= max_total_strength) {
                // Add monster to encounter
                nlohmann::json monster_info;
                monster_info["hrid"] = spawn["combatMonsterHrid"].get<std::string>();
                monster_info["eliteTier"] = spawn["eliteTier"].get<int>();
                
                encounter_hrids.push_back(monster_info);
                total_strength += strength;
              } else {
                // Exceeds max strength, stop adding monsters
                break;
              }
            }
            
            // Found a monster, stop searching
            break;
          }
        }
      }
    }
    
    // Increment encounters killed
    encounters_killed_++;
    
    // Create Monster objects from HRIDs
    std::vector<std::shared_ptr<Monster>> monsters;
    for (const auto& monster_info : encounter_hrids) {
      monsters.push_back(std::make_shared<Monster>(
          monster_info["hrid"].get<std::string>(),
          monster_info["eliteTier"].get<int>()));
    }
    
    return monsters;
  }
  
  // No valid spawn info found
  return {};
}

}  // namespace combat_simulator
