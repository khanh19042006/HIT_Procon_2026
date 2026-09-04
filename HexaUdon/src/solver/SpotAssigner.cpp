#include "solver/SpotAssigner.hpp"

int SpotAssigner::scoreSpot(
    int spotIdx,
    const std::vector<Spot>& spots,
    int distanceSteps,
    const std::set<int>& collectedBrands,
    int remainingStock
) {
    if (spotIdx < 0 || spotIdx >= static_cast<int>(spots.size())) return -1;
    if (remainingStock <= 0) return -1;

    int score = 0;

    // Brand diversity: massive bonus for new brand types
    int brand = spots[spotIdx].brand;
    if (collectedBrands.find(brand) == collectedBrands.end()) {
        score += 1000;
    }

    // Stock bonus: prefer spots with more stock
    score += remainingStock * 10;

    // Distance penalty: closer is better (subtract steps)
    score -= distanceSteps;

    return score;
}
