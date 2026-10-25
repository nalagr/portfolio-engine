#include "portfolio_engine/strategies/fixed_weight_strategy.hpp"
#include "portfolio_engine/linalg/matrix.hpp"
#include <cmath>

namespace portfolio_engine {

FixedWeightStrategy::FixedWeightStrategy(std::vector<double> target_weights,
                                         size_t rebalance_period)
    : target_weights_(std::move(target_weights)),
      rebalance_period_(rebalance_period) {}

void FixedWeightStrategy::on_step(
    size_t t,
    const std::vector<double>& current_prices,
    const std::vector<std::vector<double>>& /*price_history*/,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    if (t == 0 || t % rebalance_period_ != 0) {
        return;
    }

    stats.rebalance_events++;
    double vt = portfolio.compute_value();
    auto& stocks = portfolio.get_stocks();
    size_t n = stocks.size();

    // Construct diagonal price matrix A_t and target dollar allocation b_t
    Matrix coeff_at(n, n, 0.0);
    Matrix coeff_bt(n, 1, 0.0);

    for (size_t i = 0; i < n; ++i) {
        coeff_at.at(i, i) = current_prices[i];
        double weight = (i < target_weights_.size()) ? target_weights_[i] : (1.0 / static_cast<double>(n));
        coeff_bt.at(i, 0) = weight * vt;
    }

    // Solve A_t * x_t = b_t via matrix inverse
    Matrix inv_a = coeff_at.compute_inverse();
    Matrix xt = inv_a.mult(coeff_bt);

    std::vector<int> target_shares(n);
    for (size_t i = 0; i < n; ++i) {
        target_shares[i] = static_cast<int>(std::round(xt.at(i, 0)));
    }

    // Two-pass execution: sells first to free cash, then buys
    for (size_t i = 0; i < n; ++i) {
        if (target_shares[i] < stocks[i].shares()) {
            portfolio.rebalance(stocks[i].symbol(), target_shares[i], stats.buys, stats.sells, stats.dollar_volume);
        }
    }
    for (size_t i = 0; i < n; ++i) {
        if (target_shares[i] > stocks[i].shares()) {
            portfolio.rebalance(stocks[i].symbol(), target_shares[i], stats.buys, stats.sells, stats.dollar_volume);
        }
    }
}

} // namespace portfolio_engine
