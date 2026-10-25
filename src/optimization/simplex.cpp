#include "portfolio_engine/optimization/simplex.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace portfolio_engine {

LinearProgram::LinearProgram(const std::vector<std::vector<double>>& A,
                             const std::vector<double>& b,
                             const std::vector<double>& c)
    : A_(A), b_(b), c_(c) {
    if (A_.empty() || A_[0].empty()) {
        throw std::invalid_argument("Constraint matrix A cannot be empty");
    }
    if (A_.size() != b_.size()) {
        throw std::invalid_argument("Dimension mismatch between matrix A rows and vector b");
    }
}

void LinearProgram::setBasis(const std::vector<int>& columns) {
    basisColumns_ = columns;
    initial_basis_ = columns;
}

std::vector<int> LinearProgram::getBasicVariables() const {
    return basisColumns_;
}

std::vector<int> LinearProgram::getNonbasicVariables() const {
    std::vector<int> nonbasic;
    int total_vars = static_cast<int>(A_[0].size());
    for (int i = 0; i < total_vars; ++i) {
        bool is_basic = false;
        for (int col : basisColumns_) {
            if (i == col) {
                is_basic = true;
                break;
            }
        }
        if (!is_basic) {
            nonbasic.push_back(i);
        }
    }
    return nonbasic;
}

std::vector<std::vector<double>> LinearProgram::getBasisMatrix() const {
    int m = static_cast<int>(A_.size());
    int basis_size = static_cast<int>(basisColumns_.size());
    std::vector<std::vector<double>> basis(m, std::vector<double>(basis_size));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < basis_size; ++j) {
            basis[i][j] = A_[i][basisColumns_[j]];
        }
    }
    return basis;
}

std::vector<std::vector<double>> LinearProgram::getNonbasisMatrix() const {
    std::vector<int> nonbasic = getNonbasicVariables();
    int m = static_cast<int>(A_.size());
    int nonbasis_size = static_cast<int>(nonbasic.size());
    std::vector<std::vector<double>> nonbasis(m, std::vector<double>(nonbasis_size));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < nonbasis_size; ++j) {
            nonbasis[i][j] = A_[i][nonbasic[j]];
        }
    }
    return nonbasis;
}

std::vector<double> LinearProgram::computeBasicSolution() const {
    int m = static_cast<int>(A_.size());
    if (static_cast<int>(basisColumns_.size()) != m) {
        throw std::runtime_error("Basis size does not match number of constraints");
    }

    Matrix B(m, m);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            B.at(i, j) = A_[i][basisColumns_[j]];
        }
    }

    Matrix b_vec(m, 1);
    for (int i = 0; i < m; ++i) {
        b_vec.at(i, 0) = b_[i];
    }

    Matrix solved = B.solve(b_vec);
    std::vector<double> xB(m);
    for (int i = 0; i < m; ++i) {
        xB[i] = solved.at(i, 0);
    }
    return xB;
}

std::vector<double> LinearProgram::computeFullSolution() const {
    int total_vars = static_cast<int>(A_[0].size());
    std::vector<double> x(total_vars, 0.0);
    std::vector<double> xB = computeBasicSolution();

    for (size_t i = 0; i < basisColumns_.size(); ++i) {
        x[basisColumns_[i]] = xB[i];
    }
    return x;
}

double LinearProgram::computeObjectiveValue(const std::vector<double>& x) const {
    double val = 0.0;
    for (size_t i = 0; i < c_.size(); ++i) {
        val += c_[i] * x[i];
    }
    return val;
}

bool LinearProgram::isBasicFeasibleSolution() const {
    std::vector<double> xB = computeBasicSolution();
    for (double val : xB) {
        if (val < -1e-6) {
            return false;
        }
    }
    return true;
}

void LinearProgram::printBasis() const {
    for (int col : basisColumns_) {
        std::cout << col << " ";
    }
    std::cout << "\n";
}

void LinearProgram::buildTable() {
    int m = static_cast<int>(A_.size());
    int total_vars = static_cast<int>(A_[0].size());

    table_.assign(m + 1, std::vector<double>(total_vars + 1, 0.0));

    // Objective row (negated c)
    for (size_t j = 0; j < c_.size(); ++j) {
        table_[0][j] = -c_[j];
    }
    table_[0][total_vars] = 0.0;

    // Constraint rows
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < total_vars; ++j) {
            table_[i + 1][j] = A_[i][j];
        }
        table_[i + 1][total_vars] = b_[i];
    }

    if (!initial_basis_.empty()) {
        basisColumns_ = initial_basis_;
    } else if (basisColumns_.empty()) {
        // Default basis: try to find identity columns or use last m columns
        basisColumns_.resize(m);
        for (int i = 0; i < m; ++i) {
            basisColumns_[i] = total_vars - m + i;
        }
    }

    // Canonicalize constraints and row 0 for initial basis via Gauss-Jordan reduction:
    // Ensure column basisColumns_[i] has 1 at row i+1 and 0 in all other rows (including row 0).
    for (int i = 0; i < m && i < static_cast<int>(basisColumns_.size()); ++i) {
        int col = basisColumns_[i];
        if (col < 0 || col >= total_vars) continue;

        double pivot_val = table_[i + 1][col];
        if (std::abs(pivot_val) < 1e-12) {
            for (int k = i + 1; k < m; ++k) {
                if (std::abs(table_[k + 1][col]) > 1e-12) {
                    for (int j = 0; j <= total_vars; ++j) {
                        table_[i + 1][j] += table_[k + 1][j];
                    }
                    pivot_val = table_[i + 1][col];
                    break;
                }
            }
        }

        if (std::abs(pivot_val) > 1e-12) {
            for (int j = 0; j <= total_vars; ++j) {
                table_[i + 1][j] /= pivot_val;
            }
            for (int k = 0; k <= m; ++k) {
                if (k != i + 1) {
                    double factor = table_[k][col];
                    if (std::abs(factor) > 1e-12) {
                        for (int j = 0; j <= total_vars; ++j) {
                            table_[k][j] -= factor * table_[i + 1][j];
                        }
                    }
                }
            }
        }
    }
}

int LinearProgram::enterVar() const {
    int total_vars = static_cast<int>(A_[0].size());

    // Anti-cycling: after 20 iterations, activate Bland's smallest-index rule to break degenerate cycles
    if (iter_ >= 20) {
        for (int j = 0; j < total_vars; ++j) {
            if (table_[0][j] < -1e-9) {
                return j;
            }
        }
        return -1;
    }

    // Standard Dantzig rule (most negative reduced cost) for fast convergence
    int enter_col = -1;
    double min_val = -1e-9;
    for (int j = 0; j < total_vars; ++j) {
        if (table_[0][j] < min_val) {
            min_val = table_[0][j];
            enter_col = j;
        }
    }
    return enter_col;
}

int LinearProgram::leaveVar(int col) const {
    int m = static_cast<int>(A_.size());
    int right_col = static_cast<int>(A_[0].size());
    int leave_row = -1;
    double min_ratio = std::numeric_limits<double>::infinity();

    for (int i = 1; i <= m; ++i) {
        if (table_[i][col] > 1e-9) {
            double ratio = table_[i][right_col] / table_[i][col];
            if (ratio < min_ratio - 1e-11) {
                min_ratio = ratio;
                leave_row = i;
            } else if (std::abs(ratio - min_ratio) <= 1e-11) {
                // Bland's tie-breaking: prefer the basic variable with the smaller index
                if (leave_row != -1 && i - 1 < static_cast<int>(basisColumns_.size()) &&
                    leave_row - 1 < static_cast<int>(basisColumns_.size())) {
                    if (basisColumns_[i - 1] < basisColumns_[leave_row - 1]) {
                        leave_row = i;
                    }
                }
            }
        }
    }
    return leave_row;
}

void LinearProgram::pivot(int row, int col) {
    int m = static_cast<int>(A_.size());
    int right_col = static_cast<int>(A_[0].size());
    double pivot_val = table_[row][col];

    for (int j = 0; j <= right_col; ++j) {
        table_[row][j] /= pivot_val;
    }

    for (int i = 0; i <= m; ++i) {
        if (i != row) {
            double factor = table_[i][col];
            for (int j = 0; j <= right_col; ++j) {
                table_[i][j] -= factor * table_[row][j];
            }
        }
    }

    if (basisColumns_.size() < static_cast<size_t>(m)) {
        basisColumns_.resize(m);
    }
    basisColumns_[row - 1] = col;
}

bool LinearProgram::solve() {
    buildTable();
    iter_ = 0;
    is_optimal_ = false;
    is_unbounded_ = false;

    constexpr int MAX_ITERATIONS = 10000;
    while (iter_ < MAX_ITERATIONS) {
        int e_col = enterVar();
        if (e_col == -1) {
            is_optimal_ = true;
            return true;
        }
        int l_row = leaveVar(e_col);
        if (l_row == -1) {
            is_unbounded_ = true;
            return false;
        }
        pivot(l_row, e_col);
        iter_++;
    }
    return false;
}

double LinearProgram::getOpt() const noexcept {
    if (table_.empty()) return 0.0;
    int right_col = static_cast<int>(A_[0].size());
    return table_[0][right_col];
}

std::vector<double> LinearProgram::getSol() const {
    if (is_optimal_ && !table_.empty()) {
        int total_vars = static_cast<int>(A_[0].size());
        std::vector<double> x(total_vars, 0.0);
        int m = static_cast<int>(A_.size());
        for (int i = 0; i < m && i < static_cast<int>(basisColumns_.size()); ++i) {
            int col = basisColumns_[i];
            if (col >= 0 && col < total_vars) {
                double val = table_[i + 1][total_vars];
                x[col] = (std::abs(val) < 1e-12) ? 0.0 : val;
            }
        }
        return x;
    }
    return computeFullSolution();
}

LinearProgram LinearProgram::from_canonical_form(
    const std::vector<std::vector<double>>& A_ub,
    const std::vector<double>& b_ub,
    const std::vector<double>& c) {

    size_t m = A_ub.size();
    size_t n = A_ub[0].size();
    size_t total_vars = n + m;

    std::vector<std::vector<double>> A(m, std::vector<double>(total_vars, 0.0));
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            A[i][j] = A_ub[i][j];
        }
        A[i][n + i] = 1.0; // Slack variable
    }

    std::vector<double> full_c(total_vars, 0.0);
    for (size_t j = 0; j < c.size() && j < n; ++j) {
        full_c[j] = c[j];
    }

    LinearProgram lp(A, b_ub, full_c);
    std::vector<int> initial_basis(m);
    for (size_t i = 0; i < m; ++i) {
        initial_basis[i] = static_cast<int>(n + i);
    }
    lp.setBasis(initial_basis);
    return lp;
}

} // namespace portfolio_engine
