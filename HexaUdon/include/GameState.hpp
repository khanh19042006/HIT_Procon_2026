#pragma once

#include <vector>
#include "model/Agent.hpp"
#include "model/Traffic.hpp"

struct OtherPlayer {
    int id;
    std::vector<Agent> agents;
};

struct GameState {
    long long endsAt;
    int day;

    std::vector<Agent> agents;
    std::vector<OtherPlayer> others;
    std::vector<Traffic> traffics;
};