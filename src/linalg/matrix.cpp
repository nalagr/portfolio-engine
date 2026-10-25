#include "portfolio_engine/linalg/matrix.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace portfolio_engine {

Matrix::Matrix(size_t rows, size_t cols, double init_val)
    : rows_(rows),
      cols_(cols),
      data_(rows, std::vector<double>(cols, init_val)) {}

Matrix::Matrix(const std::vector<std::vector<double>>& data) {
    rows_ = data.size();
    cols_ = rows_ > 0 ? data[0].size() : 0;
    for (const auto& row : data) {
        if (row.size() != cols_) {
            throw std::invalid_argument("Inconsistent row widths in matrix initialization");
        }
    }
    data_ = data;
}

Matrix::Matrix(std::initializer_list<std::initializer_list<double>> init) {
    rows_ = init.size();
    cols_ = rows_ > 0 ? init.begin()->size() : 0;
    data_.reserve(rows_);
    for (const auto& row : init) {
        if (row.size() != cols_) {
            throw std::invalid_argument("Inconsistent row widths in matrix initializer list");
        }
        data_.emplace_back(row.begin(), row.end());
    }
}

double& Matrix::at(size_t r, size_t c) {
    if (r >= rows_ || c >= cols_) {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return data_[r][c];
}

const double& Matrix::at(size_t r, size_t c) const {
    if (r >= rows_ || c >= cols_) {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return data_[r][c];
}

Matrix Matrix::add(const Matrix& mat) const {
    if (rows_ != mat.rows_ || cols_ != mat.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for addition");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            result.data_[i][j] = data_[i][j] + mat.data_[i][j];
        }
    }
    return result;
}

Matrix Matrix::sub(const Matrix& mat) const {
    if (rows_ != mat.rows_ || cols_ != mat.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for subtraction");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            result.data_[i][j] = data_[i][j] - mat.data_[i][j];
        }
    }
    return result;
}

Matrix Matrix::mult(const Matrix& mat) const {
    if (cols_ != mat.rows_) {
        throw std::invalid_argument("Matrix multiplication dimension mismatch (cols != rows)");
    }
    Matrix result(rows_, mat.cols_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < mat.cols_; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < cols_; ++k) {
                sum += data_[i][k] * mat.data_[k][j];
            }
            result.data_[i][j] = sum;
        }
    }
    return result;
}

Matrix Matrix::mult(double scalar) const {
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            result.data_[i][j] = data_[i][j] * scalar;
        }
    }
    return result;
}

Matrix Matrix::trans() const {
    Matrix result(cols_, rows_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            result.data_[j][i] = data_[i][j];
        }
    }
    return result;
}

Matrix Matrix::remove_part(size_t row_rem, size_t col_rem) const {
    if (rows_ <= 1 || cols_ <= 1) {
        throw std::invalid_argument("Cannot remove part from a matrix with dimension <= 1");
    }
    if (row_rem >= rows_ || col_rem >= cols_) {
        throw std::out_of_range("Submatrix removal indices out of bounds");
    }

    Matrix reduced(rows_ - 1, cols_ - 1);
    size_t new_r = 0;
    for (size_t i = 0; i < rows_; ++i) {
        if (i == row_rem) continue;
        size_t new_c = 0;
        for (size_t j = 0; j < cols_; ++j) {
            if (j == col_rem) continue;
            reduced.data_[new_r][new_c] = data_[i][j];
            ++new_c;
        }
        ++new_r;
    }
    return reduced;
}

double Matrix::determinant() const {
    if (rows_ != cols_) {
        throw std::invalid_argument("Determinant is only defined for square matrices");
    }
    if (rows_ == 0) return 1.0;
    if (rows_ == 1) return data_[0][0];
    if (rows_ == 2) {
        return data_[0][0] * data_[1][1] - data_[0][1] * data_[1][0];
    }
    if (rows_ == 3) {
        return data_[0][0] * (data_[1][1] * data_[2][2] - data_[1][2] * data_[2][1])
             - data_[0][1] * (data_[1][0] * data_[2][2] - data_[1][2] * data_[2][0])
             + data_[0][2] * (data_[1][0] * data_[2][1] - data_[1][1] * data_[2][0]);
    }

    // For N >= 4, use Gaussian elimination with partial pivoting (O(N^3))
    size_t n = rows_;
    std::vector<std::vector<double>> a = data_;
    double det = 1.0;

    for (size_t i = 0; i < n; ++i) {
        // Find pivot
        size_t pivot_row = i;
        double max_val = std::abs(a[i][i]);
        for (size_t k = i + 1; k < n; ++k) {
            double val = std::abs(a[k][i]);
            if (val > max_val) {
                max_val = val;
                pivot_row = k;
            }
        }

        if (max_val < 1e-14) {
            return 0.0; // Singular matrix
        }

        if (pivot_row != i) {
            std::swap(a[i], a[pivot_row]);
            det = -det;
        }

        double pivot = a[i][i];
        det *= pivot;

        for (size_t k = i + 1; k < n; ++k) {
            double factor = a[k][i] / pivot;
            for (size_t j = i + 1; j < n; ++j) {
                a[k][j] -= factor * a[i][j];
            }
        }
    }
    return det;
}

Matrix Matrix::compute_cofactor() const {
    if (rows_ != cols_) {
        throw std::invalid_argument("Cofactor matrix is only defined for square matrices");
    }
    Matrix cofactors(rows_, cols_);
    if (rows_ == 1) {
        cofactors.data_[0][0] = 1.0;
        return cofactors;
    }

    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            Matrix minor_mat = remove_part(i, j);
            double sign = ((i + j) % 2 == 0) ? 1.0 : -1.0;
            cofactors.data_[i][j] = sign * minor_mat.determinant();
        }
    }
    return cofactors;
}

Matrix Matrix::compute_adjugate() const {
    return compute_cofactor().trans();
}

Matrix Matrix::compute_inverse() const {
    if (rows_ != cols_) {
        throw std::invalid_argument("Inverse is only defined for square matrices");
    }
    double det = determinant();
    if (std::abs(det) < 1e-12) {
        throw std::invalid_argument("the determinant cannot be equal to zero.");
    }

    // For small matrices (N <= 3), use exact adjugate / det matching homework
    if (rows_ <= 3) {
        Matrix adj = compute_adjugate();
        return adj * (1.0 / det);
    }

    // For N >= 4, use Gauss-Jordan elimination with partial pivoting (O(N^3))
    size_t n = rows_;
    std::vector<std::vector<double>> aug(n, std::vector<double>(2 * n, 0.0));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            aug[i][j] = data_[i][j];
        }
        aug[i][n + i] = 1.0;
    }

    for (size_t i = 0; i < n; ++i) {
        // Find pivot
        size_t pivot_row = i;
        double max_val = std::abs(aug[i][i]);
        for (size_t k = i + 1; k < n; ++k) {
            double val = std::abs(aug[k][i]);
            if (val > max_val) {
                max_val = val;
                pivot_row = k;
            }
        }

        if (max_val < 1e-14) {
            throw std::invalid_argument("the determinant cannot be equal to zero.");
        }

        if (pivot_row != i) {
            std::swap(aug[i], aug[pivot_row]);
        }

        double pivot = aug[i][i];
        for (size_t j = 0; j < 2 * n; ++j) {
            aug[i][j] /= pivot;
        }

        for (size_t k = 0; k < n; ++k) {
            if (k != i) {
                double factor = aug[k][i];
                for (size_t j = 0; j < 2 * n; ++j) {
                    aug[k][j] -= factor * aug[i][j];
                }
            }
        }
    }

    Matrix inv(n, n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            inv.data_[i][j] = aug[i][n + j];
        }
    }
    return inv;
}

Matrix Matrix::solve(const Matrix& b) const {
    if (rows_ != cols_) {
        throw std::invalid_argument("Linear system matrix must be square");
    }
    if (rows_ != b.rows_) {
        throw std::invalid_argument("Dimension mismatch between matrix rows and vector b");
    }
    Matrix inv = compute_inverse();
    return inv.mult(b);
}

void Matrix::display() const {
    std::cout << *this;
}

bool Matrix::is_approx(const Matrix& other, double tol) const noexcept {
    if (rows_ != other.rows_ || cols_ != other.cols_) return false;
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            if (std::abs(data_[i][j] - other.data_[i][j]) > tol) {
                return false;
            }
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    for (size_t i = 0; i < m.rows_; ++i) {
        os << "[ ";
        for (size_t j = 0; j < m.cols_; ++j) {
            os << std::setw(10) << std::fixed << std::setprecision(4) << m.data_[i][j] << " ";
        }
        os << "]\n";
    }
    return os;
}

Matrix Matrix::identity(size_t n) {
    Matrix res(n, n, 0.0);
    for (size_t i = 0; i < n; ++i) {
        res.data_[i][i] = 1.0;
    }
    return res;
}

Matrix Matrix::diagonal(const std::vector<double>& diag) {
    Matrix res(diag.size(), diag.size(), 0.0);
    for (size_t i = 0; i < diag.size(); ++i) {
        res.data_[i][i] = diag[i];
    }
    return res;
}

Matrix Matrix::from_vector(const std::vector<double>& v) {
    Matrix res(v.size(), 1);
    for (size_t i = 0; i < v.size(); ++i) {
        res.data_[i][0] = v[i];
    }
    return res;
}

std::vector<double> Matrix::to_vector() const {
    std::vector<double> res;
    res.reserve(rows_ * cols_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            res.push_back(data_[i][j]);
        }
    }
    return res;
}

} // namespace portfolio_engine
