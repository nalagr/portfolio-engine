#pragma once

#include "portfolio_engine/strategies/strategy.hpp"

namespace portfolio_engine {

/**
 * @brief Dynamic LP optimization rebalancing strategy replicating Homework 4 Part B.
 *        Estimates trailing lookback expected returns, solves a constrained Linear
 *        Program via the custom Tableau Simplex solver, and rebalances portfolio weights.
 */
class SimplexLPStrategy : public Strategy {
public:
    explicit SimplexLPStrategy(size_t rebalance_period = BenchmarkPreset::DEFAULT_REBALANCE_PERIOD,
                              double max_individual_weight = 0.60,
                              double max_lead_spread = 0.10);

    std::string name() const override { return "HW4_SimplexLP"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

    const std::vector<double>& last_optimal_weights() const noexcept {
        return last_optimal_weights_;
    }

private:
    size_t rebalance_period_{100};
    double max_individual_weight_{0.60};
    double max_lead_spread_{0.10};
    std::vector<double> last_optimal_weights_;
};

} // namespace portfolio_engine
