#pragma once
#include <vector>
#include <string>
#include <functional>
#include "entities/player.hpp"

enum class Rarity {
    COMMON,
    UNCOMMON,
    RARE    
};

struct Upgrade {
    std::string name;
    std::string description;
    Rarity rarity;
    std::function<void(Player&)> apply;
};
