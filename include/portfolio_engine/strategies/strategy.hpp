#pragma once

#include "portfolio_engine/core/portfolio.hpp"
#include "portfolio_engine/core/types.hpp"
#include <string>
#include <vector>

namespace portfolio_engine {

/**
 * @brief Abstract base class for all algorithmic portfolio trading & rebalancing strategies.
 */
class Strategy {
public:
    virtual ~Strategy() = default;

    /// Unique display name of the strategy
    virtual std::string name() const = 0;

    /// Invoked at each time step after the market prices update
    virtual void on_step(
        size_t t,
        const std::vector<double>& current_prices,
        const std::vector<std::vector<double>>& price_history,
        Portfolio& portfolio,
        RebalanceStats& stats) = 0;

    /// Reset internal state between simulation runs
    virtual void reset() {}
};

} // namespace portfolio_engine
