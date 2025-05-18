#pragma once
#include <string>

class Buff {
public:
    Buff(const struct BuffData& buff, int level = 1);
    
    std::string uniqueHrid;
    std::string typeHrid;
    double ratioBoost;
    double flatBoost;
    int duration;
    int startTime;
};

struct BuffData {
    std::string uniqueHrid;
    std::string typeHrid;
    double ratioBoost;
    double ratioBoostLevelBonus;
    double flatBoost;
    double flatBoostLevelBonus;
    int duration;
};
