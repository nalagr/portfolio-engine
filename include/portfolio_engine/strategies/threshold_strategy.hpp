#pragma once

#include "portfolio_engine/strategies/strategy.hpp"
#include <unordered_map>

namespace portfolio_engine {

/**
 * @brief Dynamic price threshold trading strategy replicating Homework 1.
 *        Executes discrete trades when asset price drifts beyond configurable
 *        percentages from its previous reference price.
 */
class ThresholdTradingStrategy : public Strategy {
public:
    explicit ThresholdTradingStrategy(int trade_size = 10,
                                      double buy_threshold_pct = 0.05,
                                      double sell_threshold_pct = 0.10);

    std::string name() const override { return "HW1_ThresholdTrading"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

    void reset() override;

    const std::unordered_map<std::string, int>& trade_counts() const noexcept {
        return trade_counts_;
    }

private:
    int trade_size_{10};
    double buy_threshold_pct_{0.05};
    double sell_threshold_pct_{0.10};
    std::unordered_map<std::string, int> trade_counts_;
};

} // namespace portfolio_engine
