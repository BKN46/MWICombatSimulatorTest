#pragma once

#include <map>
#include <string>
#include <vector>
#include "CombatUnit.h"
#include "ability.h"
#include "Consumable.h"
#include "Equipment.h"
#include "HouseRoom.h"

class Player : public CombatUnit {
public:
    Player();
    
    static Player createFromDTO(const json& dto);
    void updateCombatDetails() override;
    
    std::map<std::string, Equipment*> equipment;
    bool isPlayer;
    std::string hrid;
    
    std::vector<Consumable*> food;
    std::vector<Consumable*> drinks;
    std::vector<Ability*> abilities;
    std::vector<HouseRoom> houseRooms;
    
    int staminaLevel;
    int intelligenceLevel;
    int attackLevel;
    int powerLevel;
    int defenseLevel;
    int rangedLevel;
    int magicLevel;
};
