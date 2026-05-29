#!/bin/bash
# Check actual binary precision in generated ELF

echo "=== Checking Binary Precision ==="
echo ""

cat > test_simple.gd << 'EOF'
func test():
    var pi = 3.14159265358979
    return pi
EOF

echo "Single precision ELF generation:"
./build-single/Release/gdscript_to_riscv_single.exe < test_simple.gd > /dev/null 2>&1 && echo "  Success" || echo "  Failed (no objdump)"

echo ""
echo "Double precision ELF generation:"
./build-double/Release/gdscript_to_riscv_double.exe < test_simple.gd > /dev/null 2>&1 && echo "  Success" || echo "  Failed (no objdump)"

echo ""
echo "Checking if REAL_T_IS_DOUBLE is defined in binaries:"
echo ""

# Check compile definitions in the library
echo "Single precision library (should NOT have REAL_T_IS_DOUBLE):"
strings ./build-single/Release/gdscript_compiler_single.lib 2>/dev/null | grep -i "REAL_T" || echo "  No REAL_T defines found (CORRECT for single)"

echo ""
echo "Double precision library (should have REAL_T_IS_DOUBLE):"
strings ./build-double/Release/gdscript_compiler_double.lib 2>/dev/null | grep -i "REAL_T" || echo "  No REAL_T defines found (BUG - should have REAL_T_IS_DOUBLE)"

echo ""
echo "Checking math_defs.h in both builds:"
echo ""
echo "Single build compile commands:"
cat ./build-single/gdscript_compiler_single.vcxproj 2>/dev/null | grep -i "REAL_T" || echo "  Not in vcxproj"
cat ./build-single/CMakeCache.txt 2>/dev/null | grep -i "REAL_T" || echo "  Not in CMakeCache"

echo ""
echo "Double build compile commands:"
cat ./build-double/gdscript_compiler_double.vcxproj 2>/dev/null | grep -i "REAL_T" || echo "  Not in vcxproj"
cat ./build-double/CMakeCache.txt 2>/dev/null | grep -i "REAL_T" || echo "  Not in CMakeCache"

rm -f test_simple.gd
