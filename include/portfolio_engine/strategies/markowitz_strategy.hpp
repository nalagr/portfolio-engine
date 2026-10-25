#pragma once

#include "portfolio_engine/strategies/strategy.hpp"

namespace portfolio_engine {

/**
 * @brief Markowitz Mean-Variance Quadratic Programming Strategy (HW5 Part B).
 */
class MarkowitzQPStrategy : public Strategy {
public:
    explicit MarkowitzQPStrategy(double lambda_risk = 10.0,
                                 size_t rebalance_period = BenchmarkPreset::DEFAULT_REBALANCE_PERIOD,
                                 double max_weight = 0.60);

    std::string name() const override { return "HW5_MarkowitzQP"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

private:
    double lambda_risk_{10.0};
    size_t rebalance_period_{100};
    double max_weight_{0.60};
};

/**
 * @brief Integer Linear Programming Strategy with Asset Cardinality Selection (HW5 Part C).
 */
class CardinalityILPStrategy : public Strategy {
public:
    explicit CardinalityILPStrategy(int max_assets = 2,
                                   size_t rebalance_period = BenchmarkPreset::DEFAULT_REBALANCE_PERIOD);

    std::string name() const override { return "HW5_CardinalityILP"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

private:
    int max_assets_{2};
    size_t rebalance_period_{100};
};

/**
 * @brief Mixed-Integer Linear Programming Strategy with Semi-Continuous Bounds (HW5 Part D).
 */
class SemiContinuousMILPStrategy : public Strategy {
public:
    explicit SemiContinuousMILPStrategy(int max_assets = 2,
                                        double min_weight = 0.10,
                                        double max_weight = 0.60,
                                        size_t rebalance_period = BenchmarkPreset::DEFAULT_REBALANCE_PERIOD);

    std::string name() const override { return "HW5_SemiContinuousMILP"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

private:
    int max_assets_{2};
    double min_weight_{0.10};
    double max_weight_{0.60};
    size_t rebalance_period_{100};
};

/**
 * @brief Mixed-Integer Quadratic Programming Strategy with Mean-Variance and Cardinality (HW5 Part E).
 */
class MarkowitzMIQPStrategy : public Strategy {
public:
    explicit MarkowitzMIQPStrategy(double lambda_risk = 10.0,
                                   int max_assets = 2,
                                   double min_weight = 0.10,
                                   double max_weight = 0.60,
                                   size_t rebalance_period = BenchmarkPreset::DEFAULT_REBALANCE_PERIOD);

    std::string name() const override { return "HW5_MarkowitzMIQP"; }

    void on_step(size_t t,
                 const std::vector<double>& current_prices,
                 const std::vector<std::vector<double>>& price_history,
                 Portfolio& portfolio,
                 RebalanceStats& stats) override;

private:
    double lambda_risk_{10.0};
    int max_assets_{2};
    double min_weight_{0.10};
    double max_weight_{0.60};
    size_t rebalance_period_{100};
};

} // namespace portfolio_engine
