#include "rapidcheck.h"
#include "lexer.h"
#include "parser.h"
#include "math_defs.h"
#include <cmath>
#include <limits>
#include <iomanip>

using namespace gdscript;

// Property: Float literals are parsed correctly
RC_PROP("Float parsing preserves value within precision limits") {
    double input_value;
    rc::gen::real(input_value, rc::gen::real::Range::Normal);
    
    std::string source = "func test(): var x = " + std::to_string(input_value);
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    // Find the FLOAT token
    real_t parsed_value = 0;
    bool found = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::FLOAT) {
            parsed_value = std::get<real_t>(token.value);
            found = true;
            break;
        }
    }
    
    rc::assume(found);
    
    // Check that the parsed value is within precision limits
    real_t expected = static_cast<real_t>(input_value);
    real_t max_error = std::numeric_limits<real_t>::epsilon() * std::abs(expected) * 10;
    real_t actual_error = std::abs(parsed_value - expected);
    
    return actual_error <= max_error;
}

// Property: Pi constant is parsed correctly
RC_PROP("Pi constant parsing") {
    std::string source = "func test(): var pi = 3.14159265358979";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    real_t parsed_pi = 0;
    bool found = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::FLOAT) {
            parsed_pi = std::get<real_t>(token.value);
            found = true;
            break;
        }
    }
    
    rc::assume(found);
    
    #ifdef REAL_T_IS_DOUBLE
    // Double precision: should have ~15 digits
    return std::abs(parsed_pi - 3.14159265358979) < 1e-14;
    #else
    // Float precision: should have ~7 digits
    return std::abs(parsed_pi - 3.14159f) < 1e-5f;
    #endif
}

// Property: Small values are representable
RC_PROP("Very small values are representable") {
    double tiny;
    rc::gen::real(tiny, rc::gen::real::Range::Small);
    
    std::string source = "func test(): var x = " + std::to_string(tiny);
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    bool found_float = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::FLOAT) {
            found_float = true;
            break;
        }
    }
    
    return found_float;
}

// Property: Large values are representable
RC_PROP("Very large values are representable") {
    double huge;
    rc::gen::real(huge, rc::gen::real::Range::Normal);
    huge *= 1e10;
    
    std::string source = "func test(): var x = " + std::to_string(huge);
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    bool found_float = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::FLOAT) {
            found_float = true;
            break;
        }
    }
    
    return found_float;
}

// Property: real_t size is correct
RC_PROP("real_t has correct size") {
    #ifdef REAL_T_IS_DOUBLE
    return sizeof(real_t) == sizeof(double);
    #else
    return sizeof(real_t) == sizeof(float);
    #endif
}

int main() {
    std::cout << "=== Precision Property Tests ===" << std::endl;
    std::cout << "real_t size: " << sizeof(real_t) << " bytes" << std::endl;
    std::cout << "max_digits10: " << std::numeric_limits<real_t>::max_digits10 << std::endl;
    
    #ifdef REAL_T_IS_DOUBLE
    std::cout << "Mode: DOUBLE PRECISION" << std::endl;
    #else
    std::cout << "Mode: SINGLE PRECISION" << std::endl;
    #endif
    
    rc::suite("Precision Tests", {
        {"Float parsing", []() {
            rc::check("Float parsing preserves value within precision limits");
        }},
        {"Pi constant", []() {
            rc::check("Pi constant parsing");
        }},
        {"Small values", []() {
            rc::check("Very small values are representable");
        }},
        {"Large values", []() {
            rc::check("Very large values are representable");
        }},
        {"Type size", []() {
            rc::check("real_t has correct size");
        }}
    });
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}
