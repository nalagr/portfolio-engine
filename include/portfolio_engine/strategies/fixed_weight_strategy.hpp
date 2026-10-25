#pragma once

#include "portfolio_engine/strategies/strategy.hpp"

namespace portfolio_engine {

/**
 * @brief Fixed-weight asset allocation strategy replicating Homework 2 Part D.
 *        Periodically solves a diagonal linear system A_t * x_t = b_t via matrix
 *        inversion to rebalance portfolio shares to target percentages.
 */
class FixedWeightStrategy : public Strategy {
public:
    explicit FixedWeightStrategy(std::vector<double> target_weights = {0.40, 0.30, 0.30},
                                 size_t rebalance_period = BenchmarkPreset::DEFAULT_REBALANCE_PERIOD);

    std::string name() const override { return "HW2_FixedWeight"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

    const std::vector<double>& target_weights() const noexcept { return target_weights_; }

private:
    std::vector<double> target_weights_;
    size_t rebalance_period_{100};
};

} // namespace portfolio_engine
