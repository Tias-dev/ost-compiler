#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_
#include "BreakPointer.hpp"
#include <string>
namespace globals {

inline std::string foutName = "./out.tu4";
inline std::string libDir = "./";

inline bool printDebugInfo = true;
inline bool enableBreakpoints = true;
inline IBreakpointer * breakpointer = new DummyBreakpointer{};
} // namespace globals
#endif // !GLOBALS_HPP_
