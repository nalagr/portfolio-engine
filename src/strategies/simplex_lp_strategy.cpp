#include "portfolio_engine/strategies/simplex_lp_strategy.hpp"
#include "portfolio_engine/optimization/simplex.hpp"
#include <cmath>

namespace portfolio_engine {

SimplexLPStrategy::SimplexLPStrategy(size_t rebalance_period,
                                     double max_individual_weight,
                                     double max_lead_spread)
    : rebalance_period_(rebalance_period),
      max_individual_weight_(max_individual_weight),
      max_lead_spread_(max_lead_spread) {}

void SimplexLPStrategy::on_step(
    size_t t,
    const std::vector<double>& current_prices,
    const std::vector<std::vector<double>>& price_history,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    if (t == 0 || t % rebalance_period_ != 0 || t < rebalance_period_) {
        return;
    }

    stats.rebalance_events++;
    double vt = portfolio.compute_value();
    auto& stocks = portfolio.get_stocks();
    size_t n = stocks.size();

    // 1. Calculate trailing lookback expected returns
    size_t prev_t = t - rebalance_period_;
    std::vector<double> mu(n, 0.0);
    for (size_t i = 0; i < n; ++i) {
        double p_base = price_history[prev_t][i];
        double p_curr = current_prices[i];
        mu[i] = (p_base > 0.0) ? ((p_curr - p_base) / p_base) : 0.0;
    }

    // 2. Build canonical inequalities A_ub * x <= b_ub
    // Constraint 1: sum(x_i) <= 1.0 (budget)
    // Constraint 2: x_0 - x_1 <= max_lead_spread (if n >= 2)
    // Constraints 3..n+2: x_i <= max_individual_weight
    size_t m = 1 + (n >= 2 ? 1 : 0) + n;
    std::vector<std::vector<double>> A_ub(m, std::vector<double>(n, 0.0));
    std::vector<double> b_ub(m, 0.0);

    size_t row = 0;
    // Budget constraint
    for (size_t j = 0; j < n; ++j) {
        A_ub[row][j] = 1.0;
    }
    b_ub[row] = 1.00;
    ++row;

    // Spread constraint
    if (n >= 2) {
        A_ub[row][0] = 1.0;
        A_ub[row][1] = -1.0;
        b_ub[row] = max_lead_spread_;
        ++row;
    }

    // Individual asset upper bounds
    for (size_t j = 0; j < n; ++j) {
        A_ub[row][j] = 1.0;
        b_ub[row] = max_individual_weight_;
        ++row;
    }

    // 3. Convert to standard form tableau with slacks and solve
    LinearProgram lp = LinearProgram::from_canonical_form(A_ub, b_ub, mu);
    if (!lp.solve()) {
        return; // Infeasible or unbounded, preserve holdings
    }

    std::vector<double> sol = lp.getSol();
    last_optimal_weights_.assign(sol.begin(), sol.begin() + n);

    // 4. Compute target integer shares
    std::vector<int> target_shares(n, 0);
    for (size_t i = 0; i < n; ++i) {
        if (current_prices[i] > 0.0) {
            target_shares[i] = static_cast<int>((vt * last_optimal_weights_[i]) / current_prices[i]);
        }
    }

    // 5. Two-pass rebalancing: sells first to release cash, then buys
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
