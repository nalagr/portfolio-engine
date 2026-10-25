#pragma once

#include <initializer_list>
#include <iosfwd>
#include <vector>

namespace portfolio_engine {

/**
 * @brief High-performance 2D matrix class supporting exact cofactor inversion,
 *        matrix arithmetic, and linear system solving for portfolio rebalancing.
 */
class Matrix {
public:
    Matrix(size_t rows, size_t cols, double init_val = 0.0);
    Matrix(const std::vector<std::vector<double>>& data);
    Matrix(std::initializer_list<std::initializer_list<double>> init);

    // Dimensions
    size_t rows() const noexcept { return rows_; }
    size_t cols() const noexcept { return cols_; }
    int get_rows() const noexcept { return static_cast<int>(rows_); }
    int get_cols() const noexcept { return static_cast<int>(cols_); }
    int height() const noexcept { return static_cast<int>(rows_); }
    int width() const noexcept { return static_cast<int>(cols_); }

    // Element access
    double& at(size_t r, size_t c);
    const double& at(size_t r, size_t c) const;

    double& operator()(size_t r, size_t c) { return at(r, c); }
    const double& operator()(size_t r, size_t c) const { return at(r, c); }

    const std::vector<std::vector<double>>& raw_data() const noexcept { return data_; }

    // Arithmetic
    Matrix add(const Matrix& mat) const;
    Matrix sub(const Matrix& mat) const;
    Matrix mult(const Matrix& mat) const;
    Matrix mult(double scalar) const;
    Matrix trans() const;

    Matrix operator+(const Matrix& other) const { return add(other); }
    Matrix operator-(const Matrix& other) const { return sub(other); }
    Matrix operator*(const Matrix& other) const { return mult(other); }
    Matrix operator*(double scalar) const { return mult(scalar); }
    friend Matrix operator*(double scalar, const Matrix& mat) { return mat.mult(scalar); }

    // Submatrix & Inversion Operations
    Matrix remove_part(size_t row_rem, size_t col_rem) const;

    double determinant() const;
    Matrix compute_cofactor() const;
    Matrix compute_adjugate() const;
    Matrix compute_inverse() const;

    // Linear System Solving (A x = b => x = A^-1 * b)
    Matrix solve(const Matrix& b) const;

    // Output and comparison
    void display() const;
    bool is_approx(const Matrix& other, double tol = 1e-7) const noexcept;
    friend std::ostream& operator<<(std::ostream& os, const Matrix& m);

    // Static constructors
    static Matrix identity(size_t n);
    static Matrix diagonal(const std::vector<double>& diag);
    static Matrix from_vector(const std::vector<double>& v);
    std::vector<double> to_vector() const;

private:
    size_t rows_{0};
    size_t cols_{0};
    std::vector<std::vector<double>> data_;
};

} // namespace portfolio_engine
