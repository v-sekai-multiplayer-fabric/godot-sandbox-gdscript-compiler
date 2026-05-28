# Precision Support Implementation for gdscript-compiler

## Overview

This document describes how to add universal binary precision support (single=32-bit float, double=64-bit double) to the gdscript-compiler, matching Godot's precision system.

## Implementation Pattern

The implementation follows Godot's exact pattern from `core/math/math_defs.h`:

```cpp
#ifdef REAL_T_IS_DOUBLE
typedef double real_t;
#else
typedef float real_t;
#endif
```

## Files Modified

### 1. `src/math_defs.h` (NEW)
Defines the `real_t` typedef based on the `REAL_T_IS_DOUBLE` preprocessor macro.

### 2. `src/token.h`
- Added `#include "math_defs.h"`
- Changed `Token::value` variant from `double` to `real_t`

### 3. `src/lexer.h` and `src/lexer.cpp`
- Added `#include "math_defs.h"`
- Changed `add_token(TokenType type, double value)` to use `real_t`

### 4. `src/ast.h`
- Added `#include "math_defs.h"`
- `LiteralExpr::value` variant uses `real_t`
- `LiteralExpr` constructor takes `real_t` for floats

### 5. `src/ir.h`
- `IRValue` variant uses `real_t`
- `IRGlobalVar::init_value` uses `real_t`

### 6. `src/ir_interpreter.h` and `src/ir_interpreter.cpp`
- `Value` variant uses `real_t`
- Renamed `get_double()` to `get_real()` returning `real_t`

### 7. All .cpp files
Updated all `std::get<double>` to `std::get<real_t>`:
- `codegen.cpp`
- `dump_ir.cpp`
- `ir.cpp`
- `ir_optimizer.cpp`
- `parser.cpp`
- `riscv_codegen.cpp`

### 8. `src/gdscript_to_riscv.cpp`
Added Windows POSIX compatibility:
- `_popen`/`_pclose` instead of `popen`/`pclose`
- `_mktemp_s` instead of `mkstemp`
- `_unlink` instead of `unlink`

### 9. `CMakeLists.txt`
Added precision build options:

```cmake
option(GDSCRIPT_BUILD_SINGLE "Build single precision (float) variant" ON)
option(GDSCRIPT_BUILD_DOUBLE "Build double precision (double) variant" ON)

# Default precision if only one is built
if(GDSCRIPT_BUILD_DOUBLE AND NOT GDSCRIPT_BUILD_SINGLE)
  set(GDSCRIPT_DEFAULT_PRECISION "double")
elseif(GDSCRIPT_BUILD_SINGLE AND NOT GDSCRIPT_BUILD_DOUBLE)
  set(GDSCRIPT_DEFAULT_PRECISION "single")
else()
  set(GDSCRIPT_DEFAULT_PRECISION "double")
endif()
```

## Building

### Build Both Variants (Default)
```bash
cmake -B build -S .
cmake --build build --config Release
```

### Build Single Precision Only
```bash
cmake -B build-single -S . -DGDSCRIPT_BUILD_SINGLE=ON -DGDSCRIPT_BUILD_DOUBLE=OFF
cmake --build build-single --config Release
```

### Build Double Precision Only
```bash
cmake -B build-double -S . -DGDSCRIPT_BUILD_SINGLE=OFF -DGDSCRIPT_BUILD_DOUBLE=ON
cmake --build build-double --config Release
```

## Output Binaries

### Single Precision (`real_t = float`)
- `gdscript_compiler_single.lib`
- `dump_ir_single.exe`
- `gdscript_compiler_test_single.exe`
- `gdscript_to_riscv_single.exe`

### Double Precision (`real_t = double`)
- `gdscript_compiler_double.lib`
- `dump_ir_double.exe`
- `gdscript_compiler_test_double.exe`
- `gdscript_to_riscv_double.exe`

## Testing

### Single Precision Test
```bash
echo 'func test():
  var pi = 3.14159265358979
  print(pi)' | ./build-single/Release/dump_ir_single.exe
```

Output shows `LOAD_FLOAT_IMM r0, 3.14159` (5 decimal places, float precision)

### Double Precision Test
```bash
echo 'func test():
  var pi = 3.14159265358979
  print(pi)' | ./build-double/Release/dump_ir_double.exe
```

Output shows full double precision values.

## Compatibility with Godot

This implementation is designed to be compatible with Godot's precision system:

- **Godot Single Precision**: `godot.windows.editor.x86_64.exe`
  - Uses `float` (32-bit)
  - Compiled without `REAL_T_IS_DOUBLE`

- **Godot Double Precision**: `godot.windows.editor.double.x86_64.exe`
  - Uses `double` (64-bit)
  - Compiled with `REAL_T_IS_DOUBLE` defined

The gdscript-compiler can now be used with both Godot variants by selecting the appropriate precision build.

## Key Design Decisions

1. **Typedef Approach**: Uses `real_t` typedef matching Godot's pattern
2. **Separate Binaries**: Builds distinct single/double variants (not runtime switching)
3. **Suffix Naming**: Uses `_single` and `_double` suffixes for binaries
4. **Type Safety**: All floating-point literals and operations use `real_t`
5. **Windows Compatibility**: POSIX functions replaced with Windows equivalents

## Precision Differences

| Aspect | Single (float) | Double (double) |
|--------|---------------|-----------------|
| Size | 32 bits | 64 bits |
| Precision | ~7 decimal digits | ~15-17 decimal digits |
| Range | ±3.4×10³⁸ | ±1.8×10³⁰⁸ |
| Use Case | Games, real-time graphics | Scientific, high-precision math |

## Example Usage

```gdscript
# This code will use float precision with single-precision build
func calculate():
  var pi = 3.14159265358979  # Stored as 3.14159 (float)
  var result = pi * 2.0      # Float arithmetic
  return result

# Same code with double-precision build:
# pi = 3.14159265358979 (full double precision)
# result = double arithmetic
```

## Files Created/Modified Summary

- **Created**: `src/math_defs.h`
- **Modified**: 12 source files (headers and .cpp)
- **Modified**: `CMakeLists.txt`
- **Documentation**: `PRECISION.md` (triplet format examples)

## Next Steps

1. Test with actual Godot projects using both precision variants
2. Verify RISC-V code generation produces correct floating-point instructions
3. Add precision detection to automatically select matching Godot variant
4. Consider adding precision as a runtime option (optional)
