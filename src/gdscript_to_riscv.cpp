#include "compiler.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <cstdio>
#else
#include <unistd.h>
#endif

using namespace gdscript;

// Helper to run a command and capture output
std::string run_command(const char *cmd) {
#ifdef _WIN32
  FILE *pipe = _popen(cmd, "r");
#else
  FILE *pipe = popen(cmd, "r");
#endif
  if (!pipe) {
    return "Error: Failed to run command";
  }

  char buffer[4096];
  std::string result;
  try {
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
  } catch (...) {
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    throw;
  }
#ifdef _WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return result;
}

int main(int argc, char **argv) {
  std::string source;
  std::string output_function; // Function to disassemble
  std::string temp_elf = "/tmp/gdscript_temp_XXXXXX";
  bool no_optimize = false;
  bool show_program_headers = false;

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--no-opt" || arg == "--no-optimize") {
      no_optimize = true;
    } else if (arg == "-f" || arg == "--function") {
      if (i + 1 < argc) {
        output_function = argv[++i];
      }
    } else if (arg == "-l" || arg == "--program-headers") {
      show_program_headers = true;
    } else if (source.empty()) {
      source = arg;
    }
  }

  if (source.empty()) {
    // Read from stdin
    std::string line;
    while (std::getline(std::cin, line)) {
      source += line + "\n";
    }
  }

  try {
    // Create temporary file for ELF
#ifdef _WIN32
    // Windows doesn't have mkstemp, use _mktemp_s
    std::string temp_pattern = temp_elf;
    _mktemp_s(&temp_pattern[0], temp_pattern.size() + 1);
    temp_elf = temp_pattern;
#else
    mkstemp(temp_elf.data());
#endif

    // Parse and compile to ELF
    Compiler compiler;
    CompilerOptions options;
    options.output_elf = true;
    std::vector<uint8_t> elf = compiler.compile(source, options);

    // Write ELF to temp file
    {
      std::ofstream out(temp_elf, std::ios::binary);
      out.write(reinterpret_cast<const char *>(elf.data()), elf.size());
    }

    // If user requested program headers, show them and exit
    if (show_program_headers) {
      std::ostringstream cmd;
      cmd << "readelf -l " << temp_elf << " 2>&1";
      std::string output = run_command(cmd.str().c_str());
      std::cout << output;
#ifdef _WIN32
      _unlink(temp_elf.c_str());
#else
      unlink(temp_elf.c_str());
#endif
      return 0;
    }

    // Run objdump to disassemble
    std::ostringstream cmd;
    cmd << "riscv64-linux-gnu-objdump -d " << temp_elf << " 2>&1";
    std::string disasm = run_command(cmd.str().c_str());

    // Find and print the relevant function
    std::istringstream stream(disasm);
    std::string line;
    bool function_found =
        output_function.empty(); // If no function specified, print all
    bool in_function = function_found;

    while (std::getline(stream, line)) {
      // Check for function start
      if (line.find("<" + output_function + ">") != std::string::npos ||
          line.find("<" + output_function + ">") != std::string::npos) {
        in_function = true;
        function_found = true;
        std::cout << line << std::endl;
        continue;
      }

      // Print lines while in the function
      if (in_function) {
        // Stop at next function
        if (!output_function.empty() && !line.empty() && line[0] != ' ' &&
            line.find("Disassembly") == std::string::npos) {
          in_function = false;
          break;
        }
        std::cout << line << std::endl;
      }
    }

    if (!function_found) {
      std::cerr << "Warning: Function '" << output_function
                << "' not found in disassembly." << std::endl;
      std::cerr << "Available functions:" << std::endl;

      // Extract all function names
      stream = std::istringstream(disasm);
      while (std::getline(stream, line)) {
        if (line.find("<") != std::string::npos &&
            line.find(">:") != std::string::npos) {
          size_t start = line.find("<");
          size_t end = line.find(">:");
          if (start != std::string::npos && end != std::string::npos) {
            std::cerr << "  " << line.substr(start + 1, end - start - 1)
                      << std::endl;
          }
        }
      }
    }

    // Cleanup temp file
#ifdef _WIN32
    _unlink(temp_elf.c_str());
#else
    unlink(temp_elf.c_str());
#endif

    return function_found ? 0 : 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
