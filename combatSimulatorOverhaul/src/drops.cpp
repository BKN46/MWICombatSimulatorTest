#include "drops.h"

Drops::Drops(const std::string& itemHrid, double dropRate, int minCount, int maxCount, int eliteTier) :
    itemHrid(itemHrid), dropRate(dropRate), minCount(minCount), maxCount(maxCount), eliteTier(eliteTier) {
}
