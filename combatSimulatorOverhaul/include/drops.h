#pragma once
#include <string>

class Drops {
public:
    Drops(const std::string& itemHrid, double dropRate, int minCount, int maxCount, int eliteTier);
    
    std::string itemHrid;
    double dropRate;
    int minCount;
    int maxCount;
    int eliteTier;
};
