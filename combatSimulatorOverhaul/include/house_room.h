#ifndef COMBAT_SIMULATOR_OVERHAUL_HOUSE_ROOM_H_
#define COMBAT_SIMULATOR_OVERHAUL_HOUSE_ROOM_H_

#include <string>
#include <vector>
#include <memory>
#include "buff.h"

namespace combat_simulator {

class HouseRoom {
 public:
  // Constructor that takes house room id and level
  HouseRoom(const std::string& hrid, int level);

  // Default destructor
  ~HouseRoom() = default;

 private:
  std::string hrid_;
  int level_;
  std::vector<Buff> buffs_;
};

}  // namespace combat_simulator

#endif  // COMBAT_SIMULATOR_OVERHAUL_HOUSE_ROOM_H_
