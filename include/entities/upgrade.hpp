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
// Clase Upgrade que contiene el ID de la upgrade y los efectos asociados.
struct Upgrade {
    std::string name;
    std::string description;
    Rarity rarity;
    std::function<void(Player&)> apply;
};
