#pragma once

#include "portfolio_engine/strategies/strategy.hpp"

namespace portfolio_engine {

/**
 * @brief Relative investor constraint strategy replicating Homework 2 Part E.
 *        Constructs a coupled linear system expressing relative allocation views:
 *        1) Total allocation equals current portfolio wealth: sum(S_i * x_i) = V_t
 *        2) NVDA capital equals AAPL + GOOG capital: S_0*x_0 - S_1*x_1 - S_2*x_2 = 0
 *        3) AAPL capital equals 2 * GOOG capital: S_1*x_1 - 2*S_2*x_2 = 0
 */
class InvestorViewStrategy : public Strategy {
public:
    explicit InvestorViewStrategy(size_t rebalance_period = BenchmarkPreset::DEFAULT_REBALANCE_PERIOD);

    std::string name() const override { return "HW2_InvestorView"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

private:
    size_t rebalance_period_{100};
};

} // namespace portfolio_engine
