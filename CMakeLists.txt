# GDScript to RISC-V Compiler
cmake_minimum_required(VERSION 3.10)

project(gdscript_compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Floating-point precision support
# Build both single and double precision variants for universal compatibility
option(GDSCRIPT_BUILD_SINGLE "Build single precision (32-bit float) variant" ON)
option(GDSCRIPT_BUILD_DOUBLE "Build double precision (64-bit double) variant" ON)

# Default precision for the main target (used if only one is built)
set(GDSCRIPT_DEFAULT_PRECISION "double" CACHE STRING "Default precision: single or double")
set_property(CACHE GDSCRIPT_DEFAULT_PRECISION PROPERTY STRINGS "single" "double")

# Compiler sources (shared across all precision variants)
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

# Helper function to build a precision variant
function(build_precision_variant precision_suffix precision_type)
    set(precision_name "${precision_type}${precision_suffix}")
    
    # Define precision macro
    if("${precision_type}" STREQUAL "double")
        set(precision_def "-DREAL_T_IS_DOUBLE")
        set(precision_msg "double precision (64-bit)")
    else()
        set(precision_def "")
        set(precision_msg "single precision (32-bit)")
    endif()
    
    message(STATUS "Building ${precision_msg} variant: ${precision_name}")
    
    # Library
    add_library(gdscript_compiler_${precision_type} STATIC ${COMPILER_SOURCES})
    target_compile_definitions(gdscript_compiler_${precision_type} PRIVATE ${precision_def})
    target_include_directories(gdscript_compiler_${precision_type} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    
    # Test executable
    add_executable(gdscript_compiler_test_${precision_type} src/test_compiler.cpp)
    target_link_libraries(gdscript_compiler_test_${precision_type} gdscript_compiler_${precision_type})
    target_compile_definitions(gdscript_compiler_test_${precision_type} PRIVATE ${precision_def})
    
    # Dump IR utility
    add_executable(dump_ir_${precision_type} src/dump_ir.cpp)
    target_compile_definitions(dump_ir_${precision_type} PRIVATE ${precision_def})
    if(APPLE)
        target_link_libraries(dump_ir_${precision_type} gdscript_compiler_${precision_type})
    else()
        target_link_libraries(dump_ir_${precision_type} -static gdscript_compiler_${precision_type})
    endif()
    
    # GDScript to RISC-V tool
    add_executable(gdscript_to_riscv_${precision_type} src/gdscript_to_riscv.cpp)
    target_compile_definitions(gdscript_to_riscv_${precision_type} PRIVATE ${precision_def})
    if(APPLE)
        target_link_libraries(gdscript_to_riscv_${precision_type} gdscript_compiler_${precision_type})
    else()
        target_link_libraries(gdscript_to_riscv_${precision_type} -static gdscript_compiler_${precision_type})
    endif()
endfunction()

# Build precision variants
if(GDSCRIPT_BUILD_SINGLE)
    build_precision_variant("_single" "single")
endif()

if(GDSCRIPT_BUILD_DOUBLE)
    build_precision_variant("_double" "double")
endif()

# Create convenience aliases for the default precision
if(GDSCRIPT_BUILD_SINGLE AND GDSCRIPT_BUILD_DOUBLE)
    # Universal build - create symlinks or aliases
    message(STATUS "Universal build: both single and double precision variants built")
    
    # Create alias targets for convenience
    if(GDSCRIPT_DEFAULT_PRECISION STREQUAL "double")
        add_library(gdscript_compiler ALIAS gdscript_compiler_double)
        add_executable(gdscript_compiler_test ALIAS gdscript_compiler_test_double)
        add_executable(dump_ir ALIAS dump_ir_double)
        add_executable(gdscript_to_riscv ALIAS gdscript_to_riscv_double)
    else()
        add_library(gdscript_compiler ALIAS gdscript_compiler_single)
        add_executable(gdscript_compiler_test ALIAS gdscript_compiler_test_single)
        add_executable(dump_ir ALIAS dump_ir_single)
        add_executable(gdscript_to_riscv ALIAS gdscript_to_riscv_single)
    endif()
elseif(GDSCRIPT_BUILD_SINGLE)
    # Single precision only
    add_library(gdscript_compiler ALIAS gdscript_compiler_single)
    add_executable(gdscript_compiler_test ALIAS gdscript_compiler_test_single)
    add_executable(dump_ir ALIAS dump_ir_single)
    add_executable(gdscript_to_riscv ALIAS gdscript_to_riscv_single)
elseif(GDSCRIPT_BUILD_DOUBLE)
    # Double precision only
    add_library(gdscript_compiler ALIAS gdscript_compiler_double)
    add_executable(gdscript_compiler_test ALIAS gdscript_compiler_test_double)
    add_executable(dump_ir ALIAS dump_ir_double)
    add_executable(gdscript_to_riscv ALIAS gdscript_to_riscv_double)
endif()

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
if(GDSCRIPT_BUILD_SINGLE AND GDSCRIPT_BUILD_DOUBLE)
    # Install both variants in universal build
    install(TARGETS gdscript_compiler_single gdscript_compiler_double
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
    install(TARGETS gdscript_compiler_test_single gdscript_compiler_test_double
            dump_ir_single dump_ir_double
            gdscript_to_riscv_single gdscript_to_riscv_double
            RUNTIME DESTINATION bin)
elseif(GDSCRIPT_BUILD_SINGLE)
    # Install single precision only
    install(TARGETS gdscript_compiler_single gdscript_compiler_test_single
            dump_ir_single gdscript_to_riscv_single
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
elseif(GDSCRIPT_BUILD_DOUBLE)
    # Install double precision only
    install(TARGETS gdscript_compiler_double gdscript_compiler_test_double
            dump_ir_double gdscript_to_riscv_double
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()
