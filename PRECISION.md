# Precision Support

The compiler supports both single (32-bit float) and double (64-bit double) precision, matching Godot's system.

## Quick Start

```bash
# Build both variants (default)
cmake -B build -S . && cmake --build build

# Single precision only (faster, smaller)
cmake -B build -S . -DGDSCRIPT_BUILD_DOUBLE=OFF && cmake --build build

# Double precision only (high accuracy)
cmake -B build -S . -DGDSCRIPT_BUILD_SINGLE=OFF && cmake --build build
```

## Usage

```bash
# Single precision
./build-single/Release/dump_ir_single script.gd
./build-single/Release/gdscript_to_riscv_single script.gd -o out.elf

# Double precision
./build-double/Release/dump_ir_double script.gd
./build-double/Release/gdscript_to_riscv_double script.gd -o out.elf
```

## How It Works

The compiler uses `real_t` type abstraction (from `src/math_defs.h`):

```cpp
#ifdef REAL_T_IS_DOUBLE
typedef double real_t;  // 64-bit, ~15-17 decimal digits
#else
typedef float real_t;   // 32-bit, ~7 decimal digits
#endif
```

- **Single precision**: `REAL_T_IS_DOUBLE` NOT defined → `real_t = float`
- **Double precision**: `REAL_T_IS_DOUBLE` defined → `real_t = double`

## Godot Compatibility

| Godot Build | Compiler Build |
|-------------|----------------|
| `precision=single` | `GDSCRIPT_BUILD_SINGLE=ON` |
| `precision=double` | `GDSCRIPT_BUILD_DOUBLE=ON` |

## Build Options

| Option | Default | Purpose |
|--------|---------|---------|
| `GDSCRIPT_BUILD_SINGLE` | `ON` | Build 32-bit float variant |
| `GDSCRIPT_BUILD_DOUBLE` | `ON` | Build 64-bit double variant |

## Performance

| Precision | Pros | Cons |
|-----------|------|------|
| Single (float) | Faster, smaller memory | ~7 decimal digits |
| Double | ~15-17 decimal digits | Slower, larger memory |

## Files

- `src/math_defs.h` - Type definition
- `CMakeLists.txt` - Build configuration
- All source files use `real_t` instead of hardcoded `double`
