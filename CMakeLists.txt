cmake_minimum_required(VERSION 3.16)
project(PortfolioEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(ENABLE_GUROBI "Enable Gurobi solver integration for Markowitz QP, ILP, MILP, and MIQP" OFF)
option(BUILD_EXAMPLES "Build example executables" ON)
option(BUILD_TESTS "Build test suite" ON)

# Library target
add_library(portfolio_engine STATIC
    src/core/asset.cpp
    src/core/portfolio.cpp
    src/linalg/matrix.cpp
    src/optimization/simplex.cpp
    src/optimization/gurobi_solver.cpp
    src/simulation/gbm_simulator.cpp
    src/simulation/simulation_engine.cpp
    src/strategies/threshold_strategy.cpp
    src/strategies/fixed_weight_strategy.cpp
    src/strategies/investor_view_strategy.cpp
    src/strategies/simplex_lp_strategy.cpp
    src/strategies/markowitz_strategy.cpp
)

target_include_directories(portfolio_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Optional Gurobi Integration
if(ENABLE_GUROBI)
    if(DEFINED ENV{GUROBI_HOME})
        set(GUROBI_ROOT "$ENV{GUROBI_HOME}")
    else()
        set(GUROBI_ROOT "C:/gurobi1302/win64")
    endif()

    find_path(GUROBI_INCLUDE_DIR gurobi_c++.h PATHS "${GUROBI_ROOT}/include")
    find_library(GUROBI_C_LIB NAMES gurobi130 gurobi120 gurobi110 gurobi100 gurobi PATHS "${GUROBI_ROOT}/lib")
    find_library(GUROBI_CPP_LIB_RELEASE NAMES gurobi_c++md2017 gurobi_c++mt2017 gurobi_c++ PATHS "${GUROBI_ROOT}/lib")
    find_library(GUROBI_CPP_LIB_DEBUG NAMES gurobi_c++mdd2017 gurobi_c++mtd2017 gurobi_c++d PATHS "${GUROBI_ROOT}/lib")

    if(GUROBI_INCLUDE_DIR AND GUROBI_C_LIB)
        message(STATUS "Found Gurobi includes: ${GUROBI_INCLUDE_DIR}")
        message(STATUS "Found Gurobi C lib: ${GUROBI_C_LIB}")
        target_include_directories(portfolio_engine PUBLIC "${GUROBI_INCLUDE_DIR}")
        target_compile_definitions(portfolio_engine PUBLIC PORTFOLIO_ENGINE_ENABLE_GUROBI=1)
        target_link_libraries(portfolio_engine PUBLIC "${GUROBI_C_LIB}")
        if(GUROBI_CPP_LIB_DEBUG AND GUROBI_CPP_LIB_RELEASE)
            target_link_libraries(portfolio_engine PUBLIC
                $<$<CONFIG:Debug>:${GUROBI_CPP_LIB_DEBUG}>
                $<$<NOT:$<CONFIG:Debug>>:${GUROBI_CPP_LIB_RELEASE}>
            )
        elseif(GUROBI_CPP_LIB_RELEASE)
            target_link_libraries(portfolio_engine PUBLIC "${GUROBI_CPP_LIB_RELEASE}")
        endif()
    else()
        message(WARNING "Gurobi requested but not found. Disabling Gurobi integration.")
    endif()
endif()

# Examples
if(BUILD_EXAMPLES)
    add_executable(run_backtest examples/run_backtest.cpp)
    target_link_libraries(run_backtest PRIVATE portfolio_engine)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()

    add_executable(test_matrix tests/test_matrix.cpp)
    target_link_libraries(test_matrix PRIVATE portfolio_engine)
    add_test(NAME test_matrix COMMAND test_matrix)

    add_executable(test_simplex tests/test_simplex.cpp)
    target_link_libraries(test_simplex PRIVATE portfolio_engine)
    add_test(NAME test_simplex COMMAND test_simplex)

    add_executable(test_portfolio tests/test_portfolio.cpp)
    target_link_libraries(test_portfolio PRIVATE portfolio_engine)
    add_test(NAME test_portfolio COMMAND test_portfolio)
endif()
