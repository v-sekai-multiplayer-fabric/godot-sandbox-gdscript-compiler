#include "math_defs.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstring>

int main() {
    std::cout << "=== Precision Check ===" << std::endl;
    std::cout << "real_t size: " << sizeof(real_t) << " bytes" << std::endl;
    std::cout << "max_digits10: " << std::numeric_limits<real_t>::max_digits10 << std::endl;
    
    #ifdef REAL_T_IS_DOUBLE
    std::cout << "REAL_T_IS_DOUBLE: DEFINED (double precision)" << std::endl;
    #else
    std::cout << "REAL_T_IS_DOUBLE: NOT DEFINED (single precision)" << std::endl;
    #endif
    
    real_t pi = 3.14159265358979323846;
    real_t e_val = 2.71828182845904523536;
    
    std::cout << "\nPi value:" << std::endl;
    std::cout << "  Default: " << pi << std::endl;
    std::cout << "  Full precision: " << std::setprecision(std::numeric_limits<real_t>::max_digits10) << pi << std::endl;
    std::cout << "  Hex: 0x" << std::hex << *(uint32_t*)&pi << std::dec << std::endl;
    
    std::cout << "\nE value:" << std::endl;
    std::cout << "  Default: " << e_val << std::endl;
    std::cout << "  Full precision: " << std::setprecision(std::numeric_limits<real_t>::max_digits10) << e_val << std::endl;
    std::cout << "  Hex: 0x" << std::hex << *(uint32_t*)&e_val << std::dec << std::endl;
    
    return 0;
}
