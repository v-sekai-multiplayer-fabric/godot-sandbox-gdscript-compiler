#include "math_defs.h"
#include <iostream>
#include <iomanip>
#include <limits>

int main() {
    std::cout << "real_t size: " << sizeof(real_t) << " bytes" << std::endl;
    std::cout << "real_t is float: " << (sizeof(real_t) == sizeof(float) ? "YES" : "NO") << std::endl;
    std::cout << "real_t is double: " << (sizeof(real_t) == sizeof(double) ? "YES" : "NO") << std::endl;
    std::cout << "max_digits10: " << std::numeric_limits<real_t>::max_digits10 << std::endl;
    
    #ifdef REAL_T_IS_DOUBLE
    std::cout << "REAL_T_IS_DOUBLE is defined" << std::endl;
    #else
    std::cout << "REAL_T_IS_DOUBLE is NOT defined" << std::endl;
    #endif
    
    real_t pi = 3.14159265358979323846;
    std::cout << "pi = " << std::setprecision(std::numeric_limits<real_t>::max_digits10) << pi << std::endl;
    
    return 0;
}
