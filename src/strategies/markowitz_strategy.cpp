#include "portfolio_engine/strategies/markowitz_strategy.hpp"
#include "portfolio_engine/optimization/gurobi_solver.hpp"
#include "portfolio_engine/simulation/gbm_simulator.hpp"

namespace portfolio_engine {

namespace {
    void execute_rebalance(Portfolio& portfolio,
                           const std::vector<double>& weights,
                           const std::vector<double>& prices,
                           RebalanceStats& stats) {
        stats.rebalance_events++;
        portfolio.rebalance_weights(weights, prices, stats.buys, stats.sells, stats.dollar_volume);
    }
} // namespace

// ------------------- Markowitz QP -------------------
MarkowitzQPStrategy::MarkowitzQPStrategy(double lambda_risk,
                                         size_t rebalance_period,
                                         double max_weight)
    : lambda_risk_(lambda_risk),
      rebalance_period_(rebalance_period),
      max_weight_(max_weight) {}

void MarkowitzQPStrategy::on_step(
    size_t t,
    const std::vector<double>& current_prices,
    const std::vector<std::vector<double>>& price_history,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    if (!GurobiSolver::is_available()) return;
    if (t == 0 || t % rebalance_period_ != 0 || t < rebalance_period_) return;

    auto mu = GeometricBrownianMotion::compute_lookback_returns(price_history, t, rebalance_period_);
    auto cov = GeometricBrownianMotion::compute_covariance(price_history, t, rebalance_period_);

    auto weights = GurobiSolver::solve_markowitz_qp(mu, cov, lambda_risk_, max_weight_);
    execute_rebalance(portfolio, weights, current_prices, stats);
}

// ------------------- Cardinality ILP -------------------
CardinalityILPStrategy::CardinalityILPStrategy(int max_assets,
                                             size_t rebalance_period)
    : max_assets_(max_assets),
      rebalance_period_(rebalance_period) {}

void CardinalityILPStrategy::on_step(
    size_t t,
    const std::vector<double>& current_prices,
    const std::vector<std::vector<double>>& price_history,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    if (!GurobiSolver::is_available()) return;
    if (t == 0 || t % rebalance_period_ != 0 || t < rebalance_period_) return;

    auto mu = GeometricBrownianMotion::compute_lookback_returns(price_history, t, rebalance_period_);
    auto weights = GurobiSolver::solve_cardinality_ilp(mu, max_assets_);
    execute_rebalance(portfolio, weights, current_prices, stats);
}

// ------------------- Semi-Continuous MILP -------------------
SemiContinuousMILPStrategy::SemiContinuousMILPStrategy(int max_assets,
                                                     double min_weight,
                                                     double max_weight,
                                                     size_t rebalance_period)
    : max_assets_(max_assets),
      min_weight_(min_weight),
      max_weight_(max_weight),
      rebalance_period_(rebalance_period) {}

void SemiContinuousMILPStrategy::on_step(
    size_t t,
    const std::vector<double>& current_prices,
    const std::vector<std::vector<double>>& price_history,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    if (!GurobiSolver::is_available()) return;
    if (t == 0 || t % rebalance_period_ != 0 || t < rebalance_period_) return;

    auto mu = GeometricBrownianMotion::compute_lookback_returns(price_history, t, rebalance_period_);
    auto weights = GurobiSolver::solve_semi_continuous_milp(mu, max_assets_, min_weight_, max_weight_);
    execute_rebalance(portfolio, weights, current_prices, stats);
}

// ------------------- Markowitz MIQP -------------------
MarkowitzMIQPStrategy::MarkowitzMIQPStrategy(double lambda_risk,
                                             int max_assets,
                                             double min_weight,
                                             double max_weight,
                                             size_t rebalance_period)
    : lambda_risk_(lambda_risk),
      max_assets_(max_assets),
      min_weight_(min_weight),
      max_weight_(max_weight),
      rebalance_period_(rebalance_period) {}

void MarkowitzMIQPStrategy::on_step(
    size_t t,
    const std::vector<double>& current_prices,
    const std::vector<std::vector<double>>& price_history,
    Portfolio& portfolio,
    RebalanceStats& stats) {

    if (!GurobiSolver::is_available()) return;
    if (t == 0 || t % rebalance_period_ != 0 || t < rebalance_period_) return;

    auto mu = GeometricBrownianMotion::compute_lookback_returns(price_history, t, rebalance_period_);
    auto cov = GeometricBrownianMotion::compute_covariance(price_history, t, rebalance_period_);

    auto weights = GurobiSolver::solve_markowitz_miqp(mu, cov, lambda_risk_, max_assets_, min_weight_, max_weight_);
    execute_rebalance(portfolio, weights, current_prices, stats);
}

} // namespace portfolio_engine
