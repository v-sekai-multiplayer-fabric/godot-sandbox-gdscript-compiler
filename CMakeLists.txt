# GDScript to RISC-V Compiler
cmake_minimum_required(VERSION 3.10)

project(gdscript_compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler sources
set(COMPILER_SOURCES
    src/compiler_exception.cpp
    src/token.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ir.cpp
    src/ir_optimizer.cpp
    src/codegen.cpp
    src/riscv_codegen.cpp
    src/register_allocator.cpp
    src/elf_builder.cpp
    src/compiler.cpp
    src/ir_interpreter.cpp
)

# Build as library
add_library(gdscript_compiler STATIC ${COMPILER_SOURCES})
target_include_directories(gdscript_compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Build demo executable
add_executable(gdscript_compiler_test src/test_compiler.cpp)
target_link_libraries(gdscript_compiler_test gdscript_compiler)

function(add_gdscript_test test_name test_source)
	add_executable(${test_name} ${test_source})
	if(NOT WIN32)
		target_compile_options(${test_name} PUBLIC -fsanitize=address,undefined)
		target_link_libraries(${test_name} -fsanitize=address,undefined gdscript_compiler)
	else()
		target_link_libraries(${test_name} gdscript_compiler)
	endif()
	add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Dump IR utility
add_executable(dump_ir src/dump_ir.cpp)
if(APPLE)
	target_link_libraries(dump_ir gdscript_compiler)
else()
	target_link_libraries(dump_ir -static gdscript_compiler)
endif()

# GDScript to RISC-V disassembly tool
add_executable(gdscript_to_riscv src/gdscript_to_riscv.cpp)
if(APPLE)
	target_link_libraries(gdscript_to_riscv gdscript_compiler)
else()
	target_link_libraries(gdscript_to_riscv -static gdscript_compiler)
endif()

if (BUILD_TESTING)
	enable_testing()

	add_gdscript_test(test_lexer src/tests/test_lexer.cpp)
	add_gdscript_test(test_parser src/tests/test_parser.cpp)
	add_gdscript_test(test_codegen src/tests/test_codegen.cpp)
	add_gdscript_test(test_integration src/tests/test_integration.cpp)
	add_gdscript_test(test_primitives src/tests/test_primitives.cpp)
	add_gdscript_test(test_compilation src/tests/test_compilation.cpp)
	add_gdscript_test(test_constant_pool src/tests/test_constant_pool.cpp)
	add_gdscript_test(test_ir_optimizer src/tests/test_ir_optimizer.cpp)

	# Run all tests target
	add_custom_target(run_tests
		COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
		DEPENDS test_lexer test_parser test_codegen test_integration test_primitives test_compilation test_constant_pool test_ir_optimizer
		COMMENT "Running all GDScript compiler tests"
	)

	# Install test targets (only when BUILD_TESTING is ON)
	install(TARGETS test_lexer test_parser test_codegen test_integration test_primitives test_compilation test_constant_pool test_ir_optimizer
	        RUNTIME DESTINATION bin)
endif()

# Install targets (always built)
install(TARGETS gdscript_compiler gdscript_compiler_test
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
