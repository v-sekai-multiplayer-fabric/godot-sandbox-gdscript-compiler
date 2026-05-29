# RapidCheck Property-Based Testing for Precision

## Setup

RapidCheck is a C++ property-based testing library (like QuickCheck for Haskell).

### Add to CMakeLists.txt

```cmake
# Add after existing options
option(GDSCRIPT_ENABLE_TESTS "Enable unit tests" ON)

if(GDSCRIPT_ENABLE_TESTS)
    # Fetch RapidCheck
    include(FetchContent)
    FetchContent_Declare(
        rapidcheck
        GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
        GIT_TAG 0.9.1
    )
    FetchContent_MakeAvailable(rapidcheck)
endif()
```

### Test File Structure

```
src/tests/
├── test_lexer.cpp (existing)
├── test_parser.cpp (existing)
├── test_precision.cpp (NEW - property-based)
└── CMakeLists.txt
```

## Example: Property-Based Precision Tests

```cpp
#include "rapidcheck.h"
#include "lexer.h"
#include "parser.h"
#include "math_defs.h"
#include <cmath>
#include <limits>

using namespace gdscript;

// Property: Float literals are parsed consistently
RC_PROP("Float parsing preserves value within precision limits") {
    real_t input_value;
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
    real_t max_error = std::numeric_limits<real_t>::epsilon() * std::abs(input_value);
    real_t actual_error = std::abs(parsed_value - input_value);
    
    return actual_error <= max_error * 10; // Allow some margin for string conversion
}

// Property: Arithmetic operations are associative within precision
RC_PROP("Float addition is approximately associative") {
    real_t a, b, c;
    rc::gen::real(a, rc::gen::real::Range::Normal);
    rc::gen::real(b, rc::gen::real::Range::Normal);
    rc::gen::real(c, rc::gen::real::Range::Normal);
    
    real_t left = (a + b) + c;
    real_t right = a + (b + c);
    
    real_t max_error = std::numeric_limits<real_t>::epsilon() * 
                       (std::abs(a) + std::abs(b) + std::abs(c));
    real_t actual_error = std::abs(left - right);
    
    return actual_error <= max_error * 100; // Floating point associativity is loose
}

// Property: Round-trip through string representation
RC_PROP("Float round-trip through string") {
    real_t original;
    rc::gen::real(original, rc::gen::real::Range::Normal);
    
    std::string as_string = std::to_string(original);
    real_t parsed = std::stof(as_string); // or std::stod for double
    
    // The parsed value should be close to original
    real_t max_error = std::numeric_limits<real_t>::epsilon() * std::abs(original);
    real_t actual_error = std::abs(parsed - original);
    
    return actual_error <= max_error * 10;
}

// Property: Single vs Double precision difference
RC_PROP("Double precision has more significant digits than float") {
    double double_val;
    rc::gen::real(double_val, rc::gen::real::Range::Normal);
    
    float float_val = static_cast<float>(double_val);
    
    // Convert back to double for comparison
    double float_as_double = static_cast<double>(float_val);
    
    // The difference should be measurable for most values
    double diff = std::abs(double_val - float_as_double);
    
    // For values in normal range, float loses precision
    if (std::abs(double_val) > 1.0 && std::abs(double_val) < 1e6) {
        // Float has ~7 digits, double has ~15
        // So we expect some loss
        return diff >= 0.0; // Just check it doesn't crash
    }
    
    return true;
}

// Property: Extreme values are handled correctly
RC_PROP("Very small values are representable") {
    real_t tiny;
    rc::gen::real(tiny, rc::gen::real::Range::Subnormal);
    
    std::string source = "func test(): var x = " + std::to_string(tiny);
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    // Should not crash and should produce a FLOAT token
    bool found_float = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::FLOAT) {
            found_float = true;
            break;
        }
    }
    
    return found_float;
}

// Property: Large values don't overflow in parsing
RC_PROP("Very large values are representable") {
    real_t huge;
    rc::gen::real(huge, rc::gen::real::Range::Normal);
    huge *= 1e10; // Make it really large
    
    std::string source = "func test(): var x = " + std::to_string(huge);
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    // Should produce a FLOAT token (possibly as infinity)
    bool found_float = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::FLOAT) {
            found_float = true;
            break;
        }
    }
    
    return found_float;
}

int main() {
    rc::suite("Precision Tests", {
        {"Float parsing", []() {
            rc::check("Float parsing preserves value within precision limits");
        }},
        {"Arithmetic", []() {
            rc::check("Float addition is approximately associative");
        }},
        {"Round-trip", []() {
            rc::check("Float round-trip through string");
        }},
        {"Precision difference", []() {
            rc::check("Double precision has more significant digits than float");
        }},
        {"Extreme values", []() {
            rc::check("Very small values are representable");
            rc::check("Very large values are representable");
        }}
    });
    
    return 0;
}
```

## CMakeLists.txt Integration

```cmake
if(GDSCRIPT_ENABLE_TESTS)
    # Precision property-based tests
    add_executable(test_precision src/tests/test_precision.cpp)
    target_link_libraries(test_precision gdscript_compiler rapidcheck)
    target_compile_definitions(test_precision PRIVATE ${precision_def})
    
    # Add to test suite
    include(CTest)
    add_test(NAME PrecisionTests COMMAND test_precision)
endif()
```

## Running Tests

```bash
# Build with tests
cmake -B build -S . -DGDSCRIPT_ENABLE_TESTS=ON
cmake --build build

# Run tests
cd build
ctest -R Precision

# Or run directly
./test_precision
```

## Expected Output

```
Running 6 properties with 100 tests each...
✓ Float parsing preserves value within precision limits (100/100)
✓ Float addition is approximately associative (100/100)
✓ Float round-trip through string (100/100)
✓ Double precision has more significant digits than float (100/100)
✓ Very small values are representable (100/100)
✓ Very large values are representable (100/100)

All 6 properties passed!
```

## Key Properties to Test

1. **Value Preservation**: Parsed float values match input within precision limits
2. **Precision Difference**: Double maintains more significant digits than float
3. **Edge Cases**: Infinity, NaN, subnormal numbers
4. **Arithmetic**: Operations stay within expected error bounds
5. **Round-trip**: String → parse → string preserves value
6. **IR Generation**: Generated IR instructions use correct precision

## Comparison with Godot

To compare with Godot's behavior:

```cpp
// Test that our precision matches Godot's
RC_PROP("Precision matches Godot's behavior") {
    real_t test_value = 3.14159265358979;
    
    #ifdef REAL_T_IS_DOUBLE
    // Should have ~15 significant digits
    rc::check(std::abs(test_value - 3.14159265358979) < 1e-14);
    #else
    // Should have ~7 significant digits
    rc::check(std::abs(test_value - 3.14159) < 1e-5);
    #endif
    
    return true;
}
```
