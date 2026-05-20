#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_
#include <string>

#include "BreakPointer.hpp"
namespace globals {

inline std::string libDir = "./";
inline std::string outDir = "./";

inline bool printDebugInfo = false;
inline bool enableBreakpoints = false;
inline bool useBinaryFormat = false;
inline bool verboseOutput = false;
inline std::shared_ptr<IBreakpointer> breakpointer{new DummyBreakpointer{}};

inline std::string debugFirstLine = "// Compiled with breakpoints enabled";
}  // namespace globals
#endif  // !GLOBALS_HPP_
