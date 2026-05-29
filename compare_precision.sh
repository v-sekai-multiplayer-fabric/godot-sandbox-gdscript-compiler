#!/bin/bash
# 3-way comparison: Single vs Double precision

set -e

echo "=== GDScript Compiler Precision Comparison ==="
echo ""

# Test file with proper newlines
cat > test_compare.gd << 'EOF'
func test_basic():
    var pi = 3.14159265358979
    var e_val = 2.71828182845904
    var sum = pi + e_val
    var prod = pi * e_val
    return sum + prod

func test_vectors():
    var v = Vector3(1.11111111, 2.22222222, 3.33333333)
    var v2 = Vector2(0.55555555, 0.66666666)
    return v + v2

func test_extreme():
    var tiny = 0.000000000123
    var huge = 98700000000.0
    return tiny * huge
EOF

echo "1. Single Precision (float) IR:"
echo "================================"
./build-single/Release/dump_ir_single.exe < test_compare.gd 2>&1 | head -40
echo ""

echo "2. Double Precision (double) IR:"
echo "================================="
./build-double/Release/dump_ir_double.exe < test_compare.gd 2>&1 | head -40
echo ""

echo "3. Extracted float values comparison:"
echo "======================================"
echo "Single precision:"
./build-single/Release/dump_ir_single.exe < test_compare.gd 2>&1 | grep "LOAD_FLOAT_IMM" | head -10

echo ""
echo "Double precision:"
./build-double/Release/dump_ir_double.exe < test_compare.gd 2>&1 | grep "LOAD_FLOAT_IMM" | head -10

echo ""
echo "4. Binary sizes:"
echo "================"
echo "Single: $(ls -lh ./build-single/Release/dump_ir_single.exe | awk '{print $5}')"
echo "Double: $(ls -lh ./build-double/Release/dump_ir_double.exe | awk '{print $5}')"

echo ""
echo "5. Diff of IR outputs:"
echo "======================="
./build-single/Release/dump_ir_single.exe < test_compare.gd > single.ir 2>&1 || true
./build-double/Release/dump_ir_double.exe < test_compare.gd > double.ir 2>&1 || true
diff -u single.ir double.ir | head -50 || echo "(files differ)"

rm -f test_compare.gd single.ir double.ir
