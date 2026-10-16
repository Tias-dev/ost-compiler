#include "Tu4Runner.hpp"

#include <memory>
#include <string>

#include "BreakpointManager.hpp"
#include "Compiler.hpp"
#include "FilePosition.hpp"
#include "globals.hpp"
#include "trie.hpp"

std::unique_ptr<tu4run::Tu4Runner<SIZE_T, char>> tu4run::initRunner(
    const std::string& fileName, const std::string& line) {
    compiler::Commands<SIZE_T, char> commands;
    try {
        commands = compiler::serializer::deserialize(fileName,
                                                     globals::useBinaryFormat);
    } catch (compiler::serializer::NoDebugSignsException& e) {
        globals::debugMode = false;
        commands = compiler::serializer::deserialize(fileName,
                                                     globals::useBinaryFormat);
    }
    tu4run::Line<char> line_{line.c_str()};
    auto runner =
        std::make_unique<tu4run::Tu4Runner<SIZE_T, char>>(line_, commands);

    return runner;
}

tu4run::RunnerDataWithBreakpoints tu4run::initRunnerWithBreakpoints(
    const std::string& fileName, const std::string& line) {
    tu4run::RunnerDataWithBreakpoints result;
    impl::Trie<bool> usedFileNames;
    compiler::Commands<SIZE_T, char> commands;
    std::unique_ptr<LineBreakpointManager<SIZE_T>> lineBreakpointer;
    try {
        commands = compiler::serializer::deserialize(fileName,
                                                     globals::useBinaryFormat);

        const auto& fileCodes = FilePosition::fileCodesBimap().forward();
        for (const auto& entry : fileCodes)
            result.fileNames.push_back(entry.first);

        lineBreakpointer = std::make_unique<LineBreakpointManager<SIZE_T>>();
    } catch (compiler::serializer::NoDebugSignsException& e) {
        globals::debugMode = false;
        commands = compiler::serializer::deserialize(fileName,
                                                     globals::useBinaryFormat);
    }

    tu4run::Line<char> line_{line.c_str()};
    result.runner =
        std::make_unique<tu4run::Tu4Runner<SIZE_T, char>>(line_, commands);
    auto stateBreakpointer = std::make_unique<StateBreakpointManager<SIZE_T>>();

    result.breakpoints = {.stateBreakpoints = stateBreakpointer->breakpoints()};
    if (lineBreakpointer) {
        result.breakpoints.lineBreakpoints = lineBreakpointer->breakpoints();
    }
    result.runner->addBreakpointManager(std::move(stateBreakpointer));
    result.runner->addBreakpointManager(std::move(lineBreakpointer));

    return result;
}
