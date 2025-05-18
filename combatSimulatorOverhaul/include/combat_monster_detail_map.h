#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

// Forward declarations
struct CombatStats;
struct CombatDetails;
struct GameMonster;

// Function to get monster details by HRID
const GameMonster* GetCombatMonsterDetail(const std::string& hrid);

// Helper functions for stat management
void CopyStats(const CombatStats& source, CombatStats& destination);
bool HasStat(const CombatStats& stats, const std::string& stat_name);
void SetStat(CombatStats& stats, const std::string& stat_name, double value);
