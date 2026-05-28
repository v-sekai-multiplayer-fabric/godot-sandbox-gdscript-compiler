# Precision Support for GDScript Compiler

This compiler supports both single (32-bit float) and double (64-bit double) precision floating-point operations, mirroring Godot's precision system.

## Overview

The compiler uses the `real_t` type abstraction (from `math_defs.h`) which is defined as:
- `double` when `REAL_T_IS_DOUBLE` is defined (64-bit, ~15-17 decimal digits)
- `float` when `REAL_T_IS_DOUBLE` is not defined (32-bit, ~7 decimal digits)

---

## 📐 Triplet Examples

Each example follows a **Setup → Action → Result** pattern for clarity.

### Example 1: Universal Build (Default)

| **SETUP** | **ACTION** | **RESULT** |
|-----------|------------|------------|
| Fresh project directory | ```bash<br>cmake -B build -S .<br>cmake --build build<br>``` | ```<br>✓ gdscript_compiler_single.lib<br>✓ gdscript_compiler_double.lib<br>✓ dump_ir_single.exe<br>✓ dump_ir_double.exe<br>✓ gdscript_to_riscv_single.exe<br>✓ gdscript_to_riscv_double.exe<br>``` |

---

### Example 2: Single Precision Only (Performance Optimized)

| **SETUP** | **ACTION** | **RESULT** |
|-----------|------------|------------|
| Need faster, smaller binaries | ```bash<br>cmake -B build -S . \\<br>  -DGDSCRIPT_BUILD_SINGLE=ON \\<br>  -DGDSCRIPT_BUILD_DOUBLE=OFF<br>cmake --build build<br>``` | ```<br>✓ gdscript_compiler_single.lib<br>✓ dump_ir_single.exe<br>✓ gdscript_to_riscv_single.exe<br>Aliases point to _single variants<br>``` |

---

### Example 2: Double Precision Only (High Accuracy)

| **SETUP** | **ACTION** | **RESULT** |
|-----------|------------|------------|
| Need maximum precision for physics/scientific | ```bash<br>cmake -B build -S . \\<br>  -DGDSCRIPT_BUILD_SINGLE=OFF \\<br>  -DGDSCRIPT_BUILD_DOUBLE=ON<br>cmake --build build<br>``` | ```<br>✓ gdscript_compiler_double.lib<br>✓ dump_ir_double.exe<br>✓ gdscript_to_riscv_double.exe<br>Aliases point to _double variants<br>``` |

---

### Example 3: Compiling GDScript with Different Precisions

| **SETUP** | **ACTION** | **RESULT** |
|-----------|------------|------------|
| Have `script.gd` to compile | ```bash<br># Single precision<br>./build/bin/gdscript_to_riscv_single script.gd -o script_single.elf<br><br># Double precision<br>./build/bin/gdscript_to_riscv_double script.gd -o script_double.elf<br>``` | ```<br>script_single.elf - Uses fadd.s, fmul.s, etc.<br>script_double.elf - Uses fadd.d, fmul.d, etc.<br>Compare sizes: ls -lh script_*.elf<br>``` |

---

### Example 4: Inspecting IR Output

| **SETUP** | **ACTION** | **RESULT** |
|-----------|------------|------------|
| Want to see intermediate representation | ```bash<br>./build/bin/dump_ir_single script.gd<br>./build/bin/dump_ir_double script.gd<br>``` | ```<br>Single: LOAD_FLOAT_IMM r0, 3.140000f<br>Double: LOAD_FLOAT_IMM r0, 3.1400000000000001<br>Notice precision difference in literals<br>``` |

---

### Example 5: Library Integration

| **SETUP** | **ACTION** | **RESULT** |
|-----------|------------|------------|
| Building application that uses compiler | ```bash<br># Single precision app<br>g++ main.cpp -L./build/lib -lgdscript_compiler_single -o app_single<br><br># Double precision app<br>g++ main.cpp -L./build/lib -lgdscript_compiler_double -o app_double<br>``` | ```<br>app_single - Links against 32-bit float variant<br>app_double - Links against 64-bit double variant<br>Both work identically, different precision<br>``` |

---

### Example 6: Matching Godot Build

| **SETUP** | **ACTION** | **RESULT** |
|-----------|------------|------------|
| Godot built with `precision=double` | ```bash<br># Build compiler to match<br>cmake -B build -S . \\<br>  -DGDSCRIPT_BUILD_SINGLE=OFF \\<br>  -DGDSCRIPT_BUILD_DOUBLE=ON<br>cmake --build build<br><br># Compile scripts<br>./build/bin/gdscript_to_riscv script.gd -o script.elf<br>``` | ```<br>✓ Compiler uses REAL_T_IS_DOUBLE<br>✓ real_t = double (64-bit)<br>✓ Matches Godot's precision exactly<br>✓ No precision mismatch issues<br>``` |

---

## Build Options Reference

### Option Triplet: Name → Default → Purpose

| **OPTION** | **DEFAULT** | **PURPOSE** |
|------------|-------------|-------------|
| `GDSCRIPT_BUILD_SINGLE` | `ON` | Build 32-bit float variant |
| `GDSCRIPT_BUILD_DOUBLE` | `ON` | Build 64-bit double variant |
| `GDSCRIPT_DEFAULT_PRECISION` | `double` | Which variant un suffixed targets alias to |

### Configuration Matrix

| **USE CASE** | **GDSCRIPT_BUILD_SINGLE** | **GDSCRIPT_BUILD_DOUBLE** | **GDSCRIPT_DEFAULT_PRECISION** |
|--------------|---------------------------|---------------------------|--------------------------------|
| Development/Testing | `ON` | `ON` | `double` |
| Game (performance) | `ON` | `OFF` | `single` |
| Scientific/Simulation | `OFF` | `ON` | `double` |
| Match Godot single | `ON` | `OFF` | `single` |
| Match Godot double | `OFF` | `ON` | `double` |

---

## Usage

### Command Line Tools

**Single precision:**
```bash
./build/bin/dump_ir_single script.gd
./build/bin/gdscript_to_riscv_single script.gd -o output.elf
```

**Double precision:**
```bash
./build/bin/dump_ir_double script.gd
./build/bin/gdscript_to_riscv_double script.gd -o output.elf
```

### Library Usage

Link against the appropriate variant:
```bash
# Single precision
g++ main.cpp -L./build/lib -lgdscript_compiler_single

# Double precision
g++ main.cpp -L./build/lib -lgdscript_compiler_double
```

---

## Compatibility with Godot

| **GODOT SCONS** | **GDSCRIPT CMAKE** | **TYPE DEFINED** |
|-----------------|-------------------|------------------|
| `precision=single` | `GDSCRIPT_BUILD_SINGLE=ON` | `real_t = float` |
| `precision=double` | `GDSCRIPT_BUILD_DOUBLE=ON` | `real_t = double` |

When Godot is built with `precision=double`, it defines `REAL_T_IS_DOUBLE` and uses 64-bit floats for all `real_t` types. The GDScript compiler matches this behavior when built with double precision.

---

## Performance Considerations

### Single Precision (32-bit float)

| **PROS** | **CONS** |
|----------|----------|
| Faster arithmetic on most CPUs | Limited precision (~7 decimal digits) |
| Smaller memory footprint | Potential accumulation errors |
| Better cache utilization | May not match Godot double builds |
| Sufficient for most game physics | |

### Double Precision (64-bit double)

| **PROS** | **CONS** |
|----------|----------|
| Higher precision (~15-17 decimal digits) | Slower on some architectures |
| Better for scientific calculations | Larger memory usage |
| Matches Godot's `precision=double` | May be overkill for simple logic |
| Reduced numerical errors | |

---

## Type Mapping

| **GDSCRIPT TYPE** | **SINGLE PRECISION** | **DOUBLE PRECISION** |
|-------------------|----------------------|----------------------|
| `float` | `float` | `double` |
| `Vector2` | `float[2]` | `double[2]` |
| `Vector3` | `float[3]` | `double[3]` |
| `Vector4` | `float[4]` | `double[4]` |
| `Color` | `float[4]` | `double[4]` |

---

## Testing

### Test Execution Triplet

| **GOAL** | **COMMAND** | **EXPECTED** |
|----------|-------------|--------------|
| Run single precision tests | `ctest -R test_.*_single` | All tests pass with 32-bit float |
| Run double precision tests | `ctest -R test_.*_double` | All tests pass with 64-bit double |
| Run all tests | `ctest` | Both variants tested |

---

## Troubleshooting

### Issue → Diagnosis → Fix

| **ISSUE** | **DIAGNOSIS** | **FIX** |
|-----------|---------------|---------|
| "Undefined reference to real_t" | Missing include | Add `#include "math_defs.h"` |
| Mixed precision errors | Linking both variants | Build separate binaries |
| Performance regression | Accidentally using double | `cmake ... -DGDSCRIPT_BUILD_DOUBLE=OFF` |
| Precision mismatch with Godot | Different precision modes | Match Godot's `precision=` option |

---

## Technical Details

### math_defs.h Structure

```cpp
// SETUP: Conditional type definition
#ifdef REAL_T_IS_DOUBLE
typedef double real_t;  // ACTION: Use 64-bit
#else
typedef float real_t;   // ACTION: Use 32-bit
#endif

// RESULT: real_t resolves at compile time
constexpr real_t REAL_T_EPSILON = std::numeric_limits<real_t>::epsilon();
```

### IR Representation

The intermediate representation uses `real_t` for all floating-point values:
- `IRValue::fimm(real_t)` - float immediate
- `IRGlobalVar::init_value` - can hold `real_t`
- All float operations preserve precision

### RISC-V Code Generation

The code generator emits appropriate RISC-V floating-point instructions based on `real_t` size:

| **PRECISION** | **INSTRUCTIONS** |
|---------------|------------------|
| Single | `fadd.s`, `fsub.s`, `fmul.s`, `fdiv.s` |
| Double | `fadd.d`, `fsub.d`, `fmul.d`, `fdiv.d` |

---

## Quick Reference

### Most Common Workflows

| **I WANT TO** | **RUN THIS** |
|---------------|--------------|
| Build everything | `cmake -B build -S . && cmake --build build` |
| Build single only | `cmake -B build -S . -DGDSCRIPT_BUILD_DOUBLE=OFF` |
| Build double only | `cmake -B build -S . -DGDSCRIPT_BUILD_SINGLE=OFF` |
| Compile script (single) | `./build/bin/gdscript_to_riscv_single script.gd -o out.elf` |
| Compile script (double) | `./build/bin/gdscript_to_riscv_double script.gd -o out.elf` |
| View IR (compare) | `diff <(dump_ir_single s.gd) <(dump_ir_double s.gd)` |
| Run tests | `cd build && ctest` |

---

## Future Enhancements

Potential improvements:
- Runtime precision selection (currently compile-time only)
- Mixed-precision mode (different precision for different functions)
- Automatic precision optimization based on literal values
- Precision-aware IR optimizations
