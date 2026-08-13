#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"

class JsonReader {
public:
    static GameConfig readGameConfig();
    static GameState readGameState();
};