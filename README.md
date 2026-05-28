# GDScript to RISC-V Compiler

This is a complete GDScript compiler that compiles GDScript source code to RISC-V ELF binaries, executable in Godot Sandbox.

## 🎯 Quick Start

| **GOAL** | **COMMAND** | **RESULT** |
|----------|-------------|------------|
| Build compiler | ```bash<br>cmake -B build -S .<br>cmake --build build<br>``` | ✓ `gdscript_to_riscv.exe`<br>✓ `dump_ir.exe` |
| Compile script | ```bash<br>./build/bin/gdscript_to_riscv script.gd -o script.elf<br>``` | ✓ RISC-V ELF binary |
| View IR | ```bash<br>./build/bin/dump_ir script.gd<br>``` | ✓ Intermediate representation |

---

## 📐 Precision Support (New!)

The compiler now supports **both single and double precision** floating-point modes, matching Godot's precision system.

| **PRECISION** | **BUILD COMMAND** | **USE CASE** |
|---------------|-------------------|--------------|
| **Universal** (both) | `cmake -B build -S .` | Development, testing |
| **Single** (32-bit) | `cmake ... -DGDSCRIPT_BUILD_DOUBLE=OFF` | Games, performance |
| **Double** (64-bit) | `cmake ... -DGDSCRIPT_BUILD_SINGLE=OFF` | Physics, scientific |

See **[PRECISION.md](PRECISION.md)** for detailed examples and triplets.

---

## Source Files

Located in `src/`:

### Core Compiler
- **compiler.h/cpp** - Main compiler interface
- **lexer.h/cpp** - Tokenizer for GDScript
- **parser.h/cpp** - AST parser
- **ast.h** - Abstract Syntax Tree definitions
- **token.h/cpp** - Token definitions

### Code Generation Pipeline
- **codegen.h/cpp** - AST to IR (Intermediate Representation)
- **ir.h/cpp** - IR definitions
- **ir_optimizer.h/cpp** - IR optimization passes
- **ir_interpreter.h/cpp** - IR interpreter for debugging
- **riscv_codegen.h/cpp** - IR to RISC-V machine code
- **register_allocator.h/cpp** - Register allocation

### ELF Generation
- **elf_builder.h/cpp** - RISC-V ELF binary builder

### Utilities
- **compiler_exception.h/cpp** - Error handling
- **variant_types.h** - GDScript Variant type definitions
- **dump_ir.cpp** - Debug tool for IR inspection
- **gdscript_to_riscv.cpp** - Debug tool for disassembly

## API Usage

```cpp
#include <compiler.h>
using namespace gdscript;

Compiler compiler;
CompilerOptions options;
options.output_elf = true;

std::vector<uint8_t> elf_data = compiler.compile(R"(
func sum(n):
    var total = 0
    var i = 0
    while i <= n:
        total += i
        i += 1
    return total
)", options);

if (elf_data.empty()) {
    std::cerr << "Compilation failed: " << compiler.get_error() << std::endl;
}
```

## Compiler Pipeline

1. **Lexing**: GDScript source → Tokens
2. **Parsing**: Tokens → AST
3. **Code Generation**: AST → IR (Intermediate Representation)
4. **Optimization**: IR optimization passes
5. **RISC-V Codegen**: IR → RISC-V machine code
6. **ELF Building**: Machine code → Executable ELF binary

## Debug Tools

### dump_ir
Inspect the IR generated from GDScript:
```bash
cat script.gd | ./dump_ir
cat script.gd | ./dump_ir --no-optimize
cat script.gd | ./dump_ir --codegen
```

### gdscript_to_riscv
Compile and disassemble:
```bash
cat script.gd | ./gdscript_to_riscv
cat script.gd | ./gdscript_to_riscv -f function_name
```

## Features

- Full GDScript syntax support (functions, variables, control flow)
- Type hints support
- IR-based optimization
- Register allocation
- RISC-V 64-bit code generation
- ELF binary output compatible with Godot Sandbox

## License

Same as the original godot-sandbox repository.
