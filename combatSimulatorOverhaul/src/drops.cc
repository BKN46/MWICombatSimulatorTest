// Copyright 2025 BKNMWICombatSimulator
//
// Implementation of the Drops class for the combat simulator.

#include "drops.h"

#include <string>

namespace combat_simulator {

Drops::Drops(const std::string& item_hrid, 
            double drop_rate, 
            int min_count, 
            int max_count, 
            int elite_tier)
    : item_hrid_(item_hrid),
      drop_rate_(drop_rate),
      min_count_(min_count),
      max_count_(max_count),
      elite_tier_(elite_tier) {}

}  // namespace combat_simulator
