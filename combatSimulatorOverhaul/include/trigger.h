#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "combat_unit.h"

// Forward declaration
class CombatUnit;

class Trigger {
public:
    Trigger(const std::string& dependencyHrid, const std::string& conditionHrid, 
            const std::string& comparatorHrid, double value = 0);
    
    static Trigger createFromDTO(const struct TriggerDTO& dto);
    
    bool isActive(CombatUnit* source, CombatUnit* target, const std::vector<CombatUnit*>& friendlies,
                 const std::vector<CombatUnit*>& enemies, int currentTime);
    
    bool isActiveSingleTarget(CombatUnit* source, CombatUnit* target, int currentTime);
    bool isActiveMultiTarget(const std::vector<CombatUnit*>& friendlies,
                            const std::vector<CombatUnit*>& enemies, int currentTime);
    double getDependencyValue(CombatUnit* source, int currentTime);
    bool compareValue(double dependencyValue);

private:
    std::string dependencyHrid;
    std::string conditionHrid;
    std::string comparatorHrid;
    double value;
};

struct TriggerDTO {
    std::string dependencyHrid;
    std::string conditionHrid;
    std::string comparatorHrid;
    double value;
};

// Dependency map structure to be loaded from JSON
extern std::unordered_map<std::string, struct DependencyDetail> combatTriggerDependencyDetailMap;

struct DependencyDetail {
    bool isSingleTarget;
};