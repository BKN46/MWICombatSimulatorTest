#include "buff.h"

Buff::Buff(const BuffData& buff, int level) {
    uniqueHrid = buff.uniqueHrid;
    typeHrid = buff.typeHrid;
    ratioBoost = buff.ratioBoost + (level - 1) * buff.ratioBoostLevelBonus;
    flatBoost = buff.flatBoost + (level - 1) * buff.flatBoostLevelBonus;
    duration = buff.duration;
    startTime = 0;
}
