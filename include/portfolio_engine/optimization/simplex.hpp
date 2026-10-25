#pragma once

#include "portfolio_engine/linalg/matrix.hpp"
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace portfolio_engine {

/**
 * @brief High-precision Tableau Simplex Algorithm for Linear Programming.
 * 
 * Solves standard-form linear programs:
 *   maximize    c^T x
 *   subject to  A x = b
 *               x >= 0
 */
class LinearProgram {
public:
    LinearProgram(const std::vector<std::vector<double>>& A,
                  const std::vector<double>& b,
                  const std::vector<double>& c);

    // Basis configuration
    void setBasis(const std::vector<int>& columns);
    std::vector<int> getBasicVariables() const;
    std::vector<int> getNonbasicVariables() const;

    // Basis submatrices
    std::vector<std::vector<double>> getBasisMatrix() const;
    std::vector<std::vector<double>> getNonbasisMatrix() const;

    // Solution extraction
    std::vector<double> computeBasicSolution() const;
    std::vector<double> computeFullSolution() const;
    double computeObjectiveValue(const std::vector<double>& x) const;
    bool isBasicFeasibleSolution() const;
    void printBasis() const;

    // Simplex execution
    bool solve();
    int getCount() const noexcept { return iter_; }
    double getOpt() const noexcept;
    std::vector<double> getSol() const;

    bool is_optimal() const noexcept { return is_optimal_; }
    bool is_unbounded() const noexcept { return is_unbounded_; }

    // Canonical form convenience builder (A_ub * x <= b_ub with x >= 0)
    static LinearProgram from_canonical_form(
        const std::vector<std::vector<double>>& A_ub,
        const std::vector<double>& b_ub,
        const std::vector<double>& c);

private:
    void buildTable();
    int enterVar() const;
    int leaveVar(int col) const;
    void pivot(int row, int col);

    std::vector<std::vector<double>> A_;
    std::vector<double> b_;
    std::vector<double> c_;

    std::vector<int> basisColumns_;
    std::vector<int> initial_basis_;
    std::vector<std::vector<double>> table_;
    int iter_{0};
    bool is_optimal_{false};
    bool is_unbounded_{false};
};

using SimplexSolver = LinearProgram;

} // namespace portfolio_engine
