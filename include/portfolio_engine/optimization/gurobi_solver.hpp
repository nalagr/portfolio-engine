#pragma once

#include <vector>

namespace portfolio_engine {

/**
 * @brief High-level mathematical programming solver wrapping Gurobi for
 *        Markowitz QP, ILP Cardinality, MILP Semi-continuous, and MIQP models.
 */
class GurobiSolver {
public:
    static bool is_available() noexcept;

    /**
     * @brief Markowitz Mean-Variance Quadratic Program (HW5 Part B)
     *        max  mu^T x - lambda * x^T Q x
     *        s.t. sum(x) <= 1, x_0 - x_1 <= 0.10, 0 <= x_i <= max_weight
     */
    static std::vector<double> solve_markowitz_qp(
        const std::vector<double>& mu,
        const std::vector<std::vector<double>>& cov,
        double lambda_risk = 10.0,
        double max_weight = 0.60);

    /**
     * @brief Integer Linear Program Cardinality Selection (HW5 Part C)
     *        max  (1/K) * sum(mu_i * x_i)
     *        s.t. sum(x_i) <= K, x_i in {0, 1}
     */
    static std::vector<double> solve_cardinality_ilp(
        const std::vector<double>& mu,
        int max_assets = 2);

    /**
     * @brief Mixed-Integer Linear Program with Semi-Continuous Bounds (HW5 Part D)
     *        max  mu^T x
     *        s.t. sum(x) <= 1, x_0 - x_1 <= 0.10, sum(y) <= K
     *             min_w * y_i <= x_i <= max_w * y_i, y_i in {0, 1}
     */
    static std::vector<double> solve_semi_continuous_milp(
        const std::vector<double>& mu,
        int max_assets = 2,
        double min_weight = 0.10,
        double max_weight = 0.60);

    /**
     * @brief Mixed-Integer Quadratic Program (HW5 Part E)
     *        Combines Markowitz mean-variance objective with MILP cardinality bounds.
     */
    static std::vector<double> solve_markowitz_miqp(
        const std::vector<double>& mu,
        const std::vector<std::vector<double>>& cov,
        double lambda_risk = 10.0,
        int max_assets = 2,
        double min_weight = 0.10,
        double max_weight = 0.60);
};

} // namespace portfolio_engine
