# Golden Test Results - Precision Comparison

## Test File: test_precision.gd

### Single Precision (`real_t = float`)

**Output**: `test_precision_single.ir`

Key observations:
- **pi**: `3.14159` (5 decimal places)
- **e**: `2.71828` (5 decimal places)  
- **sqrt2**: `1.41421` (5 decimal places)
- **tau**: `6.28319` (5 decimal places)
- **tiny (1e-12)**: `1e-12` (scientific notation preserved)
- **large (999999999.999)**: `1e+09` (rounded to 1 billion)
- **huge (1234567890.123456)**: `1.23457e+09` (6 significant digits)
- **float_limit (1.23456789012345)**: `1.23457` (6 significant digits)
- **many_digits (0.12345678901234567890)**: `0.123457` (6 significant digits)

**Arithmetic Results (float)**:
- `pi + e = 5.85987`
- `pi - e = 0.423311`
- `pi * e = 8.53973`
- `pi / e = 1.15573`

### Double Precision (`real_t = double`)

**Output**: `test_precision_double.ir`

⚠️ **BUG DETECTED**: Double precision values are showing denormalized floats instead of correct values.

Expected vs Actual:
- **pi**: Expected `3.14159265358979`, Actual `6.98487e-315` ❌
- **e**: Expected `2.71828182845904`, Actual `1.15284e-314` ❌

This indicates a bug in the double precision build where the values are being misinterpreted. The issue is likely in how `std::stod()` returns a `double` but we're storing it in a `real_t` which is also `double` - the values should match but they're not.

**Root Cause**: The `std::stod()` function returns a `double`, and when `real_t = double`, the assignment should work. However, the output shows the bit pattern is being misinterpreted. This suggests the issue is in the IR dump printing logic or the value storage.

## Expected Golden Results

### Single Precision (float) - CORRECT
```
pi = 3.14159
e = 2.71828
pi + e = 5.85987
pi * e = 8.53973
```

### Double Precision (double) - EXPECTED
```
pi = 3.141592653589793
e = 2.718281828459045
pi + e = 5.859874482048838
pi * e = 8.539734222673567
```

## Test Commands

```bash
# Generate single precision IR
cat test_precision.gd | ./build-single/Release/dump_ir_single.exe > test_precision_single.ir

# Generate double precision IR
cat test_precision.gd | ./build-double/Release/dump_ir_double.exe > test_precision_double.ir

# Compare
diff -u test_precision_single.ir test_precision_double.ir
```

## Known Issues

1. **Double precision dump bug**: The double precision variant is showing incorrect values (denormalized floats). This needs investigation in:
   - `dump_ir.cpp` line 128-129: The `std::get<real_t>` and printing logic
   - `lexer.cpp` line 257-258: The `std::stod()` to `real_t` conversion
   - `ir.h` and `ir.cpp`: The `IRValue` variant storage

2. **Potential fix**: Ensure `std::numeric_limits<real_t>::max_digits10` is correct for both float (9) and double (17).

## Next Steps

1. Debug the double precision value storage and retrieval
2. Verify the `real_t` typedef is correctly defined when `REAL_T_IS_DOUBLE` is set
3. Check if the issue is in parsing, storage, or printing
4. Add unit tests for precision-specific behavior
