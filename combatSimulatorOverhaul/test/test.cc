#include "../include/player.h"
#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>

int main() {
  std::ifstream file("test.json");
  nlohmann::json jsonData = nlohmann::json::parse(file);
  file.close();

  auto playerData = jsonData["players"][0];
  auto player = combat_simulator::Player::CreateFromDTO(playerData);
  
  std::cout << "Player initialized successfully" << std::endl;
  return 0;
}
