#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include "solver/ActionValidator.hpp"
#include <vector>

/**
 * @brief Lop quyet dinh phan bo loai Agent (Luu thau xe)
 * 0: 巡回車 (Xe tuan tra - Patrol Car)
 * 1: 補給車 (Xe tiep te - Supply Car)
 */
class AgentStrategy {
public:
    /**
     * @brief Ham quyet dinh chon loai xe ban dau cho tat ca Agent
     * @param config Cac thong tin cau hinh ban dau (so xe, fuelLimit...)
     * @return Danh sach loai xe tuong ung (vi du: [0, 1, 0, 1])
     */
    static std::vector<int> decideAgentTypes(const GameConfig& config);
};

/**
 * @brief Lop Solver - Bo nao tinh toan va dua ra chien thuat di chuyen
 */
class Solver {
public:
    /**
     * @brief Wrapper goi sang AgentStrategy::decideAgentTypes
     */
    std::vector<int> decideAgentTypes(const GameConfig& config);

    /**
     * @brief HAM CHINH TINH TOAN LOGIC DI CHUYEN CHO CAC XE TRONG NGAY
     * 
     * @param config Cau hinh co dinh cua tran dau (ban do, spots, daySteps...)
     * @param state Trang thai cap nhat theo thoi gian thuc cua ngay nay (agent pos, fuel, traffic...)
     * @param map Doi tuong Map ho tro kiem tra dia hinh va tinh toan toa do Hex
     * @return Danh sach chuoi hanh dong cua tung Agent trong ngay
     */
    std::vector<std::vector<int>> solve(
        const GameConfig& config,
        const GameState& state,
        const Map& map
    );

    /**
     * @brief Phuong an du phong an toan (Agent dung yen cho) khi hanh dong khong hop le
     */
    std::vector<std::vector<int>> createFallbackActions(
        const GameConfig& config,
        const GameState& state
    );
};
