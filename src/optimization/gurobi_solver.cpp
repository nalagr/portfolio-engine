#include "portfolio_engine/optimization/gurobi_solver.hpp"
#include <stdexcept>

#if PORTFOLIO_ENGINE_ENABLE_GUROBI
#include "gurobi_c++.h"
#include <memory>
#include <mutex>

namespace portfolio_engine {

namespace {
    GRBEnv& get_shared_env() {
        static std::unique_ptr<GRBEnv> env;
        static std::once_flag flag;
        std::call_once(flag, []() {
            env = std::make_unique<GRBEnv>(true);
            env->set(GRB_IntParam_OutputFlag, 0);
            env->start();
        });
        return *env;
    }
} // namespace

bool GurobiSolver::is_available() noexcept {
    return true;
}

std::vector<double> GurobiSolver::solve_markowitz_qp(
    const std::vector<double>& mu,
    const std::vector<std::vector<double>>& cov,
    double lambda_risk,
    double max_weight) {

    size_t n = mu.size();
    if (n == 0 || cov.size() != n) {
        throw std::invalid_argument("Invalid dimensions for Markowitz QP");
    }

    GRBModel model(get_shared_env());
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_IntParam_NonConvex, 2);

    std::vector<GRBVar> x(n);
    for (size_t i = 0; i < n; ++i) {
        x[i] = model.addVar(0.0, max_weight, 0.0, GRB_CONTINUOUS, "x_" + std::to_string(i));
    }

    GRBLinExpr budget_expr = 0;
    for (size_t i = 0; i < n; ++i) {
        budget_expr += x[i];
    }
    model.addConstr(budget_expr <= 1.0, "budget_limit");

    if (n >= 2) {
        model.addConstr(x[0] - x[1] <= 0.10, "lead_asset_spread");
    }

    GRBQuadExpr obj = 0;
    for (size_t i = 0; i < n; ++i) {
        obj += mu[i] * x[i];
    }
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            obj -= lambda_risk * x[i] * cov[i][j] * x[j];
        }
    }

    model.setObjective(obj, GRB_MAXIMIZE);
    model.optimize();

    std::vector<double> weights(n, 0.0);
    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
        for (size_t i = 0; i < n; ++i) {
            weights[i] = x[i].get(GRB_DoubleAttr_X);
        }
    }
    return weights;
}

std::vector<double> GurobiSolver::solve_cardinality_ilp(
    const std::vector<double>& mu,
    int max_assets) {

    size_t n = mu.size();
    GRBModel model(get_shared_env());
    model.set(GRB_IntParam_OutputFlag, 0);

    std::vector<GRBVar> x(n);
    for (size_t i = 0; i < n; ++i) {
        x[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + std::to_string(i));
    }

    GRBLinExpr card_expr = 0;
    for (size_t i = 0; i < n; ++i) {
        card_expr += x[i];
    }
    model.addConstr(card_expr <= static_cast<double>(max_assets), "cardinality_limit");

    GRBLinExpr obj = 0;
    for (size_t i = 0; i < n; ++i) {
        obj += (1.0 / static_cast<double>(n)) * mu[i] * x[i];
    }
    model.setObjective(obj, GRB_MAXIMIZE);
    model.optimize();

    std::vector<double> weights(n, 0.0);
    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
        for (size_t i = 0; i < n; ++i) {
            weights[i] = x[i].get(GRB_DoubleAttr_X) / static_cast<double>(n);
        }
    }
    return weights;
}

std::vector<double> GurobiSolver::solve_semi_continuous_milp(
    const std::vector<double>& mu,
    int max_assets,
    double min_weight,
    double max_weight) {

    size_t n = mu.size();
    GRBModel model(get_shared_env());
    model.set(GRB_IntParam_OutputFlag, 0);

    std::vector<GRBVar> x(n);
    std::vector<GRBVar> y(n);
    for (size_t i = 0; i < n; ++i) {
        x[i] = model.addVar(0.0, max_weight, 0.0, GRB_CONTINUOUS, "x_" + std::to_string(i));
        y[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y_" + std::to_string(i));
    }

    GRBLinExpr budget_expr = 0;
    GRBLinExpr card_expr = 0;
    for (size_t i = 0; i < n; ++i) {
        budget_expr += x[i];
        card_expr += y[i];
    }
    model.addConstr(budget_expr <= 1.0, "budget_limit");
    model.addConstr(card_expr <= static_cast<double>(max_assets), "cardinality_limit");

    if (n >= 2) {
        model.addConstr(x[0] - x[1] <= 0.10, "lead_asset_spread");
    }

    for (size_t i = 0; i < n; ++i) {
        model.addConstr(x[i] >= min_weight * y[i], "semi_cont_lower_" + std::to_string(i));
        model.addConstr(x[i] <= max_weight * y[i], "semi_cont_upper_" + std::to_string(i));
    }

    GRBLinExpr obj = 0;
    for (size_t i = 0; i < n; ++i) {
        obj += mu[i] * x[i];
    }
    model.setObjective(obj, GRB_MAXIMIZE);
    model.optimize();

    std::vector<double> weights(n, 0.0);
    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
        for (size_t i = 0; i < n; ++i) {
            weights[i] = x[i].get(GRB_DoubleAttr_X);
        }
    }
    return weights;
}

std::vector<double> GurobiSolver::solve_markowitz_miqp(
    const std::vector<double>& mu,
    const std::vector<std::vector<double>>& cov,
    double lambda_risk,
    int max_assets,
    double min_weight,
    double max_weight) {

    size_t n = mu.size();
    GRBModel model(get_shared_env());
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_IntParam_NonConvex, 2);

    std::vector<GRBVar> x(n);
    std::vector<GRBVar> y(n);
    for (size_t i = 0; i < n; ++i) {
        x[i] = model.addVar(0.0, max_weight, 0.0, GRB_CONTINUOUS, "x_" + std::to_string(i));
        y[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y_" + std::to_string(i));
    }

    GRBLinExpr budget_expr = 0;
    GRBLinExpr card_expr = 0;
    for (size_t i = 0; i < n; ++i) {
        budget_expr += x[i];
        card_expr += y[i];
    }
    model.addConstr(budget_expr <= 1.0, "budget_limit");
    model.addConstr(card_expr <= static_cast<double>(max_assets), "cardinality_limit");

    if (n >= 2) {
        model.addConstr(x[0] - x[1] <= 0.10, "lead_asset_spread");
    }

    for (size_t i = 0; i < n; ++i) {
        model.addConstr(x[i] >= min_weight * y[i], "semi_cont_lower_" + std::to_string(i));
        model.addConstr(x[i] <= max_weight * y[i], "semi_cont_upper_" + std::to_string(i));
    }

    GRBQuadExpr obj = 0;
    for (size_t i = 0; i < n; ++i) {
        obj += mu[i] * x[i];
    }
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            obj -= lambda_risk * x[i] * cov[i][j] * x[j];
        }
    }

    model.setObjective(obj, GRB_MAXIMIZE);
    model.optimize();

    std::vector<double> weights(n, 0.0);
    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
        for (size_t i = 0; i < n; ++i) {
            weights[i] = x[i].get(GRB_DoubleAttr_X);
        }
    }
    return weights;
}

} // namespace portfolio_engine

#else // PORTFOLIO_ENGINE_ENABLE_GUROBI == 0

namespace portfolio_engine {

bool GurobiSolver::is_available() noexcept {
    return false;
}

std::vector<double> GurobiSolver::solve_markowitz_qp(
    const std::vector<double>&,
    const std::vector<std::vector<double>>&,
    double, double) {
    throw std::runtime_error("Gurobi is disabled in this build. Rebuild with -DENABLE_GUROBI=ON.");
}

std::vector<double> GurobiSolver::solve_cardinality_ilp(
    const std::vector<double>&, int) {
    throw std::runtime_error("Gurobi is disabled in this build. Rebuild with -DENABLE_GUROBI=ON.");
}

std::vector<double> GurobiSolver::solve_semi_continuous_milp(
    const std::vector<double>&, int, double, double) {
    throw std::runtime_error("Gurobi is disabled in this build. Rebuild with -DENABLE_GUROBI=ON.");
}

std::vector<double> GurobiSolver::solve_markowitz_miqp(
    const std::vector<double>&,
    const std::vector<std::vector<double>>&,
    double, int, double, double) {
    throw std::runtime_error("Gurobi is disabled in this build. Rebuild with -DENABLE_GUROBI=ON.");
}

} // namespace portfolio_engine

#endif
