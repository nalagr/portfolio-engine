#ifdef NDEBUG
#undef NDEBUG
#endif
#include "portfolio_engine/linalg/matrix.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace portfolio_engine;

void test_basic_arithmetic() {
    std::cout << "[Test Matrix] Running test_basic_arithmetic...\n";
    Matrix A = {{1.0, 2.0}, {3.0, 4.0}};
    Matrix B = {{5.0, 6.0}, {7.0, 8.0}};

    Matrix sum = A + B;
    assert(std::abs(sum.at(0, 0) - 6.0) < 1e-9);
    assert(std::abs(sum.at(0, 1) - 8.0) < 1e-9);
    assert(std::abs(sum.at(1, 0) - 10.0) < 1e-9);
    assert(std::abs(sum.at(1, 1) - 12.0) < 1e-9);

    Matrix diff = B - A;
    assert(std::abs(diff.at(0, 0) - 4.0) < 1e-9);
    assert(std::abs(diff.at(1, 1) - 4.0) < 1e-9);

    Matrix prod = A * B;
    // [1*5 + 2*7, 1*6 + 2*8] = [19, 22]
    // [3*5 + 4*7, 3*6 + 4*8] = [43, 50]
    assert(std::abs(prod.at(0, 0) - 19.0) < 1e-9);
    assert(std::abs(prod.at(0, 1) - 22.0) < 1e-9);
    assert(std::abs(prod.at(1, 0) - 43.0) < 1e-9);
    assert(std::abs(prod.at(1, 1) - 50.0) < 1e-9);

    // Dimension mismatch checks
    Matrix C(3, 2);
    bool caught_dim_err = false;
    try {
        A.add(C);
    } catch (const std::invalid_argument&) {
        caught_dim_err = true;
    }
    assert(caught_dim_err);
    std::cout << "  -> test_basic_arithmetic PASSED\n";
}

void test_determinant_and_inverse() {
    std::cout << "[Test Matrix] Running test_determinant_and_inverse...\n";
    // 1x1
    Matrix M1 = {{7.5}};
    assert(std::abs(M1.determinant() - 7.5) < 1e-9);

    // 2x2
    Matrix M2 = {{2.0, 3.0}, {1.0, 4.0}};
    assert(std::abs(M2.determinant() - 5.0) < 1e-9);

    // 3x3
    Matrix M3 = {
        {1.0, 2.0, 3.0},
        {0.0, 1.0, 4.0},
        {5.0, 6.0, 0.0}
    };
    // det = 1*(0-24) - 2*(0-20) + 3*(0-5) = -24 + 40 - 15 = 1.0
    double det = M3.determinant();
    assert(std::abs(det - 1.0) < 1e-9);

    // Inverse check M3 * M3^-1 = I
    Matrix inv = M3.compute_inverse();
    Matrix ident = M3 * inv;
    Matrix expected_ident = Matrix::identity(3);
    assert(ident.is_approx(expected_ident, 1e-7));

    // Singular matrix exception
    Matrix singular = {{1.0, 2.0}, {2.0, 4.0}};
    bool caught_singular = false;
    try {
        singular.compute_inverse();
    } catch (const std::invalid_argument&) {
        caught_singular = true;
    }
    assert(caught_singular);
    std::cout << "  -> test_determinant_and_inverse PASSED\n";
}

void test_linear_system_solve() {
    std::cout << "[Test Matrix] Running test_linear_system_solve...\n";
    // Replicate HW2 linear system for NVDA, AAPL, GOOG rebalancing
    double p0 = 100.0, p1 = 200.0, p2 = 300.0;
    double vt = 100000.0;

    Matrix A(3, 3);
    A.at(0, 0) = p0;
    A.at(1, 1) = p1;
    A.at(2, 2) = p2;

    Matrix b(3, 1);
    b.at(0, 0) = 0.40 * vt;
    b.at(1, 0) = 0.30 * vt;
    b.at(2, 0) = 0.30 * vt;

    Matrix x = A.solve(b);
    assert(std::abs(x.at(0, 0) - 400.0) < 1e-9); // 40000 / 100 = 400
    assert(std::abs(x.at(1, 0) - 150.0) < 1e-9); // 30000 / 200 = 150
    assert(std::abs(x.at(2, 0) - 100.0) < 1e-9); // 30000 / 300 = 100

    std::cout << "  -> test_linear_system_solve PASSED\n";
}

void test_arbitrary_n_matrix_scaling() {
    std::cout << "[Test Matrix] Running test_arbitrary_n_matrix_scaling (N=4, 5, 8, 10)...\n";
    // 4x4 known determinant
    // Triangular matrix with 2, 3, 4, 5 on diagonal => det = 120.0
    Matrix M4 = {
        {2.0, 1.0, 0.0, 0.0},
        {0.0, 3.0, 1.0, 0.0},
        {0.0, 0.0, 4.0, 1.0},
        {0.0, 0.0, 0.0, 5.0}
    };
    assert(std::abs(M4.determinant() - 120.0) < 1e-9);

    Matrix inv4 = M4.compute_inverse();
    Matrix ident4 = M4 * inv4;
    assert(ident4.is_approx(Matrix::identity(4), 1e-7));

    // 5x5 Linear system solve: A * x = b
    Matrix M5 = Matrix::identity(5);
    for (size_t i = 0; i < 5; ++i) {
        M5.at(i, i) = static_cast<double>(i + 1);
        if (i + 1 < 5) M5.at(i, i + 1) = 0.5;
    }
    Matrix b5(5, 1, 10.0);
    Matrix x5 = M5.solve(b5);
    Matrix check_b5 = M5 * x5;
    assert(check_b5.is_approx(b5, 1e-6));

    // 8x8 matrix inverse: A * A^-1 = I
    constexpr size_t N8 = 8;
    Matrix M8(N8, N8, 0.0);
    for (size_t i = 0; i < N8; ++i) {
        for (size_t j = 0; j < N8; ++j) {
            M8.at(i, j) = (i == j) ? 4.0 : 1.0 / static_cast<double>(i + j + 2);
        }
    }
    Matrix inv8 = M8.compute_inverse();
    Matrix ident8 = M8 * inv8;
    assert(ident8.is_approx(Matrix::identity(N8), 1e-6));

    // 10x10 matrix scaling: instant execution (O(N^3))
    constexpr size_t N10 = 10;
    Matrix M10(N10, N10, 0.0);
    for (size_t i = 0; i < N10; ++i) {
        M10.at(i, i) = 10.0 + static_cast<double>(i);
        if (i > 0) M10.at(i, i - 1) = 1.0;
        if (i + 1 < N10) M10.at(i, i + 1) = 1.0;
    }
    double det10 = M10.determinant();
    assert(std::abs(det10) > 1e-3);
    Matrix inv10 = M10.compute_inverse();
    Matrix ident10 = M10 * inv10;
    assert(ident10.is_approx(Matrix::identity(N10), 1e-6));

    std::cout << "  -> test_arbitrary_n_matrix_scaling PASSED\n";
}

int main() {
    std::cout << "Running Portfolio Engine Matrix Test Suite...\n";
    test_basic_arithmetic();
    test_determinant_and_inverse();
    test_linear_system_solve();
    test_arbitrary_n_matrix_scaling();
    std::cout << "ALL MATRIX TESTS PASSED!\n";
    return 0;
}
