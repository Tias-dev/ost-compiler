#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_
#include "BreakPointer.hpp"
#include <string>
namespace globals {

inline std::string libDir = "./";
inline std::string outDir = "./";

inline bool printDebugInfo = false;
inline bool enableBreakpoints = false;
inline bool useBinaryFormat = false;
inline IBreakpointer * breakpointer = new DummyBreakpointer{};

inline std::string debugFirstLine = "// Compiled with breakpoints enabled";
} // namespace globals
#endif // !GLOBALS_HPP_
