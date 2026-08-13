#pragma once

#include <vector>
#include <cstdint>

struct Spot {
    int brand;
    int pos;
    int stocks;
};

struct MapConfig {
    int height;
    int width;
    std::vector<std::vector<int>> cells;
};

struct GameConfig {
    long long startsAt;

    std::vector<int> daySeconds;
    std::vector<int> daySteps;

    MapConfig map;

    std::vector<Spot> spots;
    std::vector<int> initialAgentPositions;

    int fuelLimit;

    int players;
    int busyThreshold;
    int jammedThreshold;
};