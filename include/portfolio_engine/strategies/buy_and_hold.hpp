#pragma once

#include "portfolio_engine/strategies/strategy.hpp"

namespace portfolio_engine {

/**
 * @brief Passive Buy-and-Hold benchmark strategy (no rebalancing trades).
 */
class BuyAndHoldStrategy : public Strategy {
public:
    std::string name() const override { return "BuyAndHold"; }

    void on_step(size_t /*t*/,
                 const std::vector<double>& /*current_prices*/,
                 const std::vector<std::vector<double>>& /*price_history*/,
                 Portfolio& /*portfolio*/,
                 RebalanceStats& /*stats*/) override {
        // Passive hold: no trading
    }
};

} // namespace portfolio_engine
