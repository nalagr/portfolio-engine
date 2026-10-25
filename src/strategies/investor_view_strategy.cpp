#include "portfolio_engine/strategies/investor_view_strategy.hpp"
#include "portfolio_engine/linalg/matrix.hpp"
#include <cmath>

namespace portfolio_engine {

InvestorViewStrategy::InvestorViewStrategy(size_t rebalance_period)
    : rebalance_period_(rebalance_period) {}

void InvestorViewStrategy::on_step(
    size_t t,
    const std::vector<double>& current_prices,
    const std::vector<std::vector<double>>& /*price_history*/,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    if (t == 0 || t % rebalance_period_ != 0) {
        return;
    }

    auto& stocks = portfolio.get_stocks();
    size_t n = stocks.size();
    if (n < 3) return; // Requires at least 3 assets for this specific view system

    stats.rebalance_events++;
    double vt = portfolio.compute_value();

    double p0 = current_prices[0];
    double p1 = current_prices[1];
    double p2 = current_prices[2];

    Matrix coeff_a(3, 3);
    coeff_a.at(0, 0) = p0;
    coeff_a.at(0, 1) = p1;
    coeff_a.at(0, 2) = p2;

    coeff_a.at(1, 0) = p0;
    coeff_a.at(1, 1) = -p1;
    coeff_a.at(1, 2) = -p2;

    coeff_a.at(2, 0) = 0.0;
    coeff_a.at(2, 1) = p1;
    coeff_a.at(2, 2) = -2.0 * p2;

    Matrix coeff_b(3, 1);
    coeff_b.at(0, 0) = vt;
    coeff_b.at(1, 0) = 0.0;
    coeff_b.at(2, 0) = 0.0;

    Matrix inv_a = coeff_a.compute_inverse();
    Matrix xt = inv_a.mult(coeff_b);

    std::vector<int> target_shares(3);
    for (size_t i = 0; i < 3; ++i) {
        target_shares[i] = static_cast<int>(std::round(xt.at(i, 0)));
    }

    // Two-pass execution: sell first, then buy
    for (size_t i = 0; i < 3; ++i) {
        if (target_shares[i] < stocks[i].shares()) {
            portfolio.rebalance(stocks[i].symbol(), target_shares[i], stats.buys, stats.sells, stats.dollar_volume);
        }
    }
    for (size_t i = 0; i < 3; ++i) {
        if (target_shares[i] > stocks[i].shares()) {
            portfolio.rebalance(stocks[i].symbol(), target_shares[i], stats.buys, stats.sells, stats.dollar_volume);
        }
    }
}

} // namespace portfolio_engine
