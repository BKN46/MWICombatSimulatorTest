#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "ability.h"
#include "combat_unit.h"
#include "drops.h"

class Monster : public CombatUnit {
public:
    // Constructors
    Monster(const std::string& hrid, int elite_tier = 0);

    // Methods
    void UpdateCombatDetails() override;

private:
    int elite_tier_ = 0;
};
