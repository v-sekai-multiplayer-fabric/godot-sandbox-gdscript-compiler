#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

/**
 * The "Real" type is an abstract type used for real numbers, such as 1.5,
 * in contrast to integer numbers. Precision can be controlled with the
 * presence or absence of the REAL_T_IS_DOUBLE define.
 *
 * This mirrors Godot's core/math/math_defs.h for compatibility.
 */
#ifdef REAL_T_IS_DOUBLE
typedef double real_t;
#else
typedef float real_t;
#endif

// Constants for real_t using std::numeric_limits
constexpr real_t REAL_T_EPSILON = std::numeric_limits<real_t>::epsilon();
constexpr real_t REAL_T_MAX = std::numeric_limits<real_t>::max();
constexpr real_t REAL_T_MIN = std::numeric_limits<real_t>::min();

// Helper functions for real_t
inline real_t real_abs(real_t v) {
    return std::abs(v);
}

inline real_t real_sqrt(real_t v) {
    return std::sqrt(v);
}

inline real_t real_sin(real_t v) {
    return std::sin(v);
}

inline real_t real_cos(real_t v) {
    return std::cos(v);
}

inline real_t real_tan(real_t v) {
    return std::tan(v);
}

inline real_t real_asin(real_t v) {
    return std::asin(v);
}

inline real_t real_acos(real_t v) {
    return std::acos(v);
}

inline real_t real_atan(real_t v) {
    return std::atan(v);
}

inline real_t real_atan2(real_t y, real_t x) {
    return std::atan2(y, x);
}

inline real_t real_exp(real_t v) {
    return std::exp(v);
}

inline real_t real_log(real_t v) {
    return std::log(v);
}

inline real_t real_log2(real_t v) {
    return std::log2(v);
}

inline real_t real_pow(real_t base, real_t exp) {
    return std::pow(base, exp);
}

inline bool real_is_nan(real_t v) {
    return std::isnan(v);
}

inline bool real_is_inf(real_t v) {
    return std::isinf(v);
}

inline bool real_is_finite(real_t v) {
    return std::isfinite(v);
}
