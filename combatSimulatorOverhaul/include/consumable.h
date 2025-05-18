#pragma once
#include <string>
#include <vector>
#include "buff.h"
#include "trigger.h"

class Consumable {
public:
    Consumable(const std::string& hrid, const std::vector<Trigger*>* triggers = nullptr);
    ~Consumable();
    
    static Consumable* createFromDTO(const struct ConsumableDTO& dto);
    bool shouldTrigger(int currentTime, CombatUnit* source, CombatUnit* target,
                       const std::vector<CombatUnit*>& friendlies, 
                       const std::vector<CombatUnit*>& enemies);
                       
    std::string hrid;
    int cooldownDuration;
    int hitpointRestore;
    int manapointRestore;
    int recoveryDuration;
    std::string categoryHrid;
    std::vector<Buff*> buffs;
    std::vector<Trigger*> triggers;
    int lastUsed;
};

struct ConsumableDTO {
    std::string hrid;
    std::vector<TriggerDTO> triggers;
};

// Item details structure to be loaded from JSON
extern std::unordered_map<std::string, struct ItemDetail> itemDetailMap;

struct ItemDetail {
    std::string categoryHrid;
    struct ConsumableDetail {
        int cooldownDuration;
        int hitpointRestore;
        int manapointRestore;
        int recoveryDuration;
        std::vector<BuffData> buffs;
        std::vector<TriggerDTO> defaultCombatTriggers;
    } consumableDetail;
};
