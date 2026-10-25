#ifdef NDEBUG
#undef NDEBUG
#endif
#include "portfolio_engine/optimization/simplex.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace portfolio_engine;

void test_hw4_part_a() {
    std::cout << "[Test Simplex] Running test_hw4_part_a...\n";
    // HW4 Part A canonical test problem
    // max  3*x1 + 2*x2
    // s.t. x1 + x2 + s1 = 4
    //      x1 + s2 = 2
    //      x2 + s3 = 3
    std::vector<std::vector<double>> A = {
        {1.0, 1.0, 1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0, 1.0, 0.0},
        {0.0, 1.0, 0.0, 0.0, 1.0}
    };
    std::vector<double> b = {4.0, 2.0, 3.0};
    std::vector<double> c = {3.0, 2.0, 0.0, 0.0, 0.0};

    LinearProgram lp(A, b, c);
    lp.setBasis({2, 3, 4});

    // Check basis queries
    auto basic_vars = lp.getBasicVariables();
    assert(basic_vars.size() == 3);
    assert(basic_vars[0] == 2 && basic_vars[1] == 3 && basic_vars[2] == 4);

    auto nonbasic_vars = lp.getNonbasicVariables();
    assert(nonbasic_vars.size() == 2);
    assert(nonbasic_vars[0] == 0 && nonbasic_vars[1] == 1);

    // Initial BFS check
    assert(lp.isBasicFeasibleSolution());
    auto init_sol = lp.computeBasicSolution();
    assert(std::abs(init_sol[0] - 4.0) < 1e-9);
    assert(std::abs(init_sol[1] - 2.0) < 1e-9);
    assert(std::abs(init_sol[2] - 3.0) < 1e-9);

    // Solve simplex
    bool solved = lp.solve();
    assert(solved);
    assert(lp.is_optimal());

    std::vector<double> x = lp.getSol();
    // Optimal: x1 = 2, x2 = 2, s1 = 0, s2 = 0, s3 = 1
    assert(std::abs(x[0] - 2.0) < 1e-7);
    assert(std::abs(x[1] - 2.0) < 1e-7);
    assert(std::abs(x[2] - 0.0) < 1e-7);
    assert(std::abs(x[3] - 0.0) < 1e-7);
    assert(std::abs(x[4] - 1.0) < 1e-7);

    // Optimal objective = 3*2 + 2*2 = 10.0
    double opt_val = lp.getOpt();
    assert(std::abs(opt_val - 10.0) < 1e-7);
    std::cout << "  -> Optimal objective value: " << opt_val << " (iterations: " << lp.getCount() << ")\n";
    std::cout << "  -> test_hw4_part_a PASSED\n";
}

void test_canonical_form_builder() {
    std::cout << "[Test Simplex] Running test_canonical_form_builder...\n";
    // Same problem in canonical form: A_ub * x <= b_ub
    std::vector<std::vector<double>> A_ub = {
        {1.0, 1.0},
        {1.0, 0.0},
        {0.0, 1.0}
    };
    std::vector<double> b_ub = {4.0, 2.0, 3.0};
    std::vector<double> c = {3.0, 2.0};

    LinearProgram lp = LinearProgram::from_canonical_form(A_ub, b_ub, c);
    bool solved = lp.solve();
    assert(solved);
    assert(std::abs(lp.getOpt() - 10.0) < 1e-7);
    auto sol = lp.getSol();
    assert(std::abs(sol[0] - 2.0) < 1e-7);
    assert(std::abs(sol[1] - 2.0) < 1e-7);
    std::cout << "  -> test_canonical_form_builder PASSED\n";
}

void test_portfolio_rebalance_lp() {
    std::cout << "[Test Simplex] Running test_portfolio_rebalance_lp...\n";
    // HW4 Part B formulation test
    std::vector<std::vector<double>> A_lp = {
        {1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0},
        {1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}
    };
    std::vector<double> b_lp = {1.00, 0.10, 0.60, 0.60, 0.60};
    std::vector<double> c_lp = {0.05, 0.02, 0.01, 0.0, 0.0, 0.0, 0.0, 0.0};

    LinearProgram lp(A_lp, b_lp, c_lp);
    lp.setBasis({3, 4, 5, 6, 7});

    bool solved = lp.solve();
    assert(solved);

    std::vector<double> sol = lp.getSol();
    double x_nvda = sol[0];
    double x_aapl = sol[1];
    double x_goog = sol[2];

    // Check constraints
    assert(x_nvda + x_aapl + x_goog <= 1.0 + 1e-7);
    assert(x_nvda - x_aapl <= 0.10 + 1e-7);
    assert(x_nvda <= 0.60 + 1e-7);
    assert(x_aapl <= 0.60 + 1e-7);
    assert(x_goog <= 0.60 + 1e-7);
    assert(x_nvda >= -1e-7 && x_aapl >= -1e-7 && x_goog >= -1e-7);

    std::cout << "  -> Solved weights: NVDA=" << x_nvda << ", AAPL=" << x_aapl << ", GOOG=" << x_goog << "\n";
    std::cout << "  -> test_portfolio_rebalance_lp PASSED\n";
}

void test_multi_asset_simplex_lp() {
    std::cout << "[Test Simplex] Running test_multi_asset_simplex_lp (N=5 assets)...\n";
    // 5 assets: max sum(mu_i * x_i) s.t. sum(x_i) <= 1, x_0 - x_1 <= 0.1, x_i <= 0.4
    size_t n = 5;
    std::vector<double> mu = {0.08, 0.05, 0.03, 0.02, 0.01};
    size_t m = 1 + 1 + n; // 7 constraints
    std::vector<std::vector<double>> A_ub(m, std::vector<double>(n, 0.0));
    std::vector<double> b_ub(m, 0.0);

    // Budget
    for (size_t j = 0; j < n; ++j) A_ub[0][j] = 1.0;
    b_ub[0] = 1.0;

    // Spread
    A_ub[1][0] = 1.0;
    A_ub[1][1] = -1.0;
    b_ub[1] = 0.10;

    // Individual caps
    for (size_t j = 0; j < n; ++j) {
        A_ub[2 + j][j] = 1.0;
        b_ub[2 + j] = 0.40;
    }

    LinearProgram lp = LinearProgram::from_canonical_form(A_ub, b_ub, mu);
    bool solved = lp.solve();
    assert(solved);
    assert(lp.is_optimal());

    std::vector<double> sol = lp.getSol();
    double sum_w = 0.0;
    for (size_t j = 0; j < n; ++j) {
        assert(sol[j] >= -1e-7);
        assert(sol[j] <= 0.40 + 1e-7);
        sum_w += sol[j];
    }
    assert(sum_w <= 1.0 + 1e-7);
    assert(sol[0] - sol[1] <= 0.10 + 1e-7);

    // Optimal allocation allocates to highest return assets under caps:
    // x0 = 0.40 (cap), x1 = 0.40 (cap, since x0 - x1 = 0.00 <= 0.10), x2 = 0.20 (fills remaining 1.0 budget)
    // Objective: 0.08*0.40 + 0.05*0.40 + 0.03*0.20 = 0.032 + 0.020 + 0.006 = 0.058
    assert(std::abs(sol[0] - 0.40) < 1e-5);
    assert(std::abs(sol[1] - 0.40) < 1e-5);
    assert(std::abs(sol[2] - 0.20) < 1e-5);
    assert(std::abs(lp.getOpt() - 0.058) < 1e-5);

    std::cout << "  -> test_multi_asset_simplex_lp PASSED (Optimal: " << lp.getOpt() << ")\n";
}

void test_degenerate_cycling_bland() {
    std::cout << "[Test Simplex] Running test_degenerate_cycling_bland...\n";
    // Degenerate LP with ratio ties
    // max  10*x1 - 57*x2 - 9*x3 - 24*x4
    // s.t. 0.5*x1 - 5.5*x2 - 2.5*x3 + 9*x4 + s1 = 0
    //      0.5*x1 - 1.5*x2 - 0.5*x3 +   x4 + s2 = 0
    //          x1                           + s3 = 1
    std::vector<std::vector<double>> A = {
        {0.5, -5.5, -2.5,  9.0, 1.0, 0.0, 0.0},
        {0.5, -1.5, -0.5,  1.0, 0.0, 1.0, 0.0},
        {1.0,  0.0,  0.0,  0.0, 0.0, 0.0, 1.0}
    };
    std::vector<double> b = {0.0, 0.0, 1.0};
    std::vector<double> c = {10.0, -57.0, -9.0, -24.0, 0.0, 0.0, 0.0};

    LinearProgram lp(A, b, c);
    lp.setBasis({4, 5, 6});

    bool solved = lp.solve();
    assert(solved);
    assert(lp.is_optimal());
    assert(lp.getOpt() >= 0.0);
    std::cout << "  -> test_degenerate_cycling_bland PASSED (iterations: " << lp.getCount() << ")\n";
}

void test_nonzero_initial_objective_basis() {
    std::cout << "[Test Simplex] Running test_nonzero_initial_objective_basis...\n";
    // Standard form LP where initial basis contains variables with non-zero c
    // max  2*x1 + 3*x2
    // s.t. x1 + x2 = 5
    //      x1      = 2
    // With basis {1, 0} (x2 and x1 basic)
    std::vector<std::vector<double>> A = {
        {1.0, 1.0},
        {1.0, 0.0}
    };
    std::vector<double> b = {5.0, 2.0};
    std::vector<double> c = {2.0, 3.0};

    LinearProgram lp(A, b, c);
    lp.setBasis({1, 0});

    bool solved = lp.solve();
    assert(solved);
    assert(lp.is_optimal());
    auto sol = lp.getSol();
    assert(std::abs(sol[0] - 2.0) < 1e-7); // x1 = 2
    assert(std::abs(sol[1] - 3.0) < 1e-7); // x2 = 3
    assert(std::abs(lp.getOpt() - 13.0) < 1e-7); // 2*2 + 3*3 = 13

    std::cout << "  -> test_nonzero_initial_objective_basis PASSED\n";
}

int main() {
    std::cout << "Running Portfolio Engine Simplex Test Suite...\n";
    test_hw4_part_a();
    test_canonical_form_builder();
    test_portfolio_rebalance_lp();
    test_multi_asset_simplex_lp();
    test_degenerate_cycling_bland();
    test_nonzero_initial_objective_basis();
    std::cout << "ALL SIMPLEX TESTS PASSED!\n";
    return 0;
}
