#include "AST.hpp"
#include "BreakPointer.hpp"
#include "CharStream.hpp"
#include "Compiler.hpp"
#include "FilePosition.hpp"
#include "Tokenizer.hpp"
#include "globals.hpp"
#include "trie.hpp"
#include "utils.hpp"
#include <bits/getopt_core.h>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <ratio>
#include <stdexcept>
#include <string>
#include <thread>
const char *helpMessage =
    "Ost program builder\n"
    "usage: ostbuild <program>.ost [OPTIONS]\n"
    "Description:\n"
    "Used for building programs that have multiple libs\n"
    "if compiled lib is older that actual lib source so lib is rebuilded"
    "\n"
    "Awailable options are shown below:\n"
    "-s, --sources : comma separated paths to places where sourse files(.ost) "
    "are stored\n"
    "\t(requires argument, default: empty)\n"
    "-f, --force : force recompile all"
    "\t(default: false)\n"
    "-l, --libdir : directory where precompiled libraries(.tu4, not source "
    ".ost files!) are stored, rebuilded libraries will be stored here\n"
    "\t(requires argument, default: ./)\n"
    "-o, --outputdir : directory where compiled <program>.ost program will be "
    "stored\n"
    "\t(requires argument, default: ./)\n"
    "-g, --enable-breakpoints : enabling breakpoints setting option. Program "
    "compiled with this flag can be debugged with ostdb utility\n"
    "\t(default: disabled)\n"
    "-d, --print-debug-info : enabling debug info\n"
    "\t(default: disabled)\n"
    "-b, --use-binary-format : save compiled sequence of commands in binary "
    "format. Significant speed up to loading and saving\n"
    "\t(default: disabled)\n"
    "-h, --help : print this help message and quit\n"
    "\n";

using duration_t = std::chrono::milliseconds;
const char *durationSuffix = "ms";

std::set<std::string> sourcePaths;
std::string sources = "";

void fillSources(const std::string &s);
void parseCommandArgs(int argc, char *argw[]);
void compileMainProgram(const std::string &filename);

int main(int argc, char *argw[]) {
  if (argc < 2)
    throw std::invalid_argument("Usage: ostbuild <program>.ost [OPTIONS]");
  std::string fileName = argw[1];
  parseCommandArgs(argc, argw);

  if (globals::outDir[globals::outDir.size() - 1] != '/')
    globals::outDir.push_back('/');

  if (globals::libDir[globals::libDir.size() - 1] != '/')
    globals::libDir.push_back('/');
  if (globals::enableBreakpoints)
    globals::breakpointer =
        std::shared_ptr<IBreakpointer>{new FileBreakpointer()};

  if (!sources.empty())
    fillSources(sources);

  auto start_ts = std::chrono::system_clock::now();
  compileMainProgram(fileName);
  auto end_ts = std::chrono::system_clock::now();
  logger::debug() << "BUILDER: total building time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         end_ts - start_ts)
                         .count()
                  << "ms";

  return 0;
}

size_t nWorkers = 1;
bool forceRecompile = false;
void parseCommandArgs(int argc, char *argw[]) {
  size_t nopts = 9;
  option *options = new option[nopts]{
      {.name = "libdir", .has_arg = 1, .flag = NULL, .val = 'l'},
      {.name = "outputdir", .has_arg = 1, .flag = NULL, .val = 'o'},
      {.name = "enable-breakpoints", .has_arg = 0, .flag = NULL, .val = 'g'},
      {.name = "print-debug-info", .has_arg = 0, .flag = NULL, .val = 'd'},
      {.name = "force", .has_arg = 0, .flag = NULL, .val = 'f'},
      {.name = "use-binary-format", .has_arg = 0, .flag = NULL, .val = 'b'},
      {.name = "sources", .has_arg = 1, .flag = NULL, .val = 's'},
      {.name = "jobs", .has_arg = 1, .flag = NULL, .val = 'j'},
      {.name = "help", .has_arg = 0, .flag = NULL, .val = 'h'}};
  memset(&options[nopts - 1], 0, sizeof(option));

  int arg, longindex;
  while ((arg = getopt_long(argc, argw, "l:o:gdbhs:j:f", options,
                            &longindex)) != -1) {
    switch (arg) {
    case '?':
      logger::warning() << "Unrecognized option: " << optarg << std::endl;
      break;
    case 'l':
      globals::libDir = optarg;
      logger::info() << "Used library directory: " << optarg;
      break;
    case 'o':
      globals::outDir = optarg;
      logger::info() << "Used output directory: " << optarg;
      break;
    case 'g':
      globals::enableBreakpoints = true;
      logger::info() << "Breakpoints enabled";
      break;
    case 'd':
      globals::printDebugInfo = true;
      logger::info() << "Printing debug info enabled";
      break;
    case 'b':
      globals::useBinaryFormat = true;
      logger::info() << "Binary format to saving enabled";
      break;
    case 's':
      logger::info() << "Additional sources are added";
      sources = std::string(optarg);
      break;
    case 'f':
      logger::info() << "Force recompile enabled";
      forceRecompile = true;
      break;
    case 'j':
      nWorkers = atoll(optarg);
      logger::info() << "Using " << nWorkers << " jobs";
      break;
    case 'h':
      std::cout << helpMessage << std::endl;
      exit(0);
      break;
    default:
      logger::warning() << "Given option: [" << char(arg)
                        << "] can't be processed";
    }
  }
  delete[] options;
}

/**
 * @brief Creates ast tree from program in file
 *
 * @param fileName -- path to program
 *
 * @return ast Tree
 */
ast::Tree toAST(const std::string &fileName) {
  ast::Tree::clearNamesTable();
  static token::Tokenizer tokenizer;
  std::ifstream file(fileName);
  if (!file.is_open())
    throw std::invalid_argument(strfast() << "Can't open file: " << fileName);
  CharStream stream(file);
  FileRoller roller(std::make_shared<std::string>(fileName));
  auto tokens = tokenizer.parse(stream, roller);
  ast::Tree ast{tokens, fileName};
  {
    logger::debug out;
    out << "Detected libs for [" << fileName << "]:\n";
    for (const auto &lib : ast.libs)
      out << lib << '\n';
  }
  return ast;
}

struct ThrowSourceNotFoundError {
  ThrowSourceNotFoundError(const std::string &mt) {
    throw std::invalid_argument(
        strfast()
        << "Source(.ost) file for mt [" << mt
        << "] not found in source directories\n"
        << "Please ensure that it exist in directories specified by -s flag");
  }
};

using mt_name_t = std::string;
using is_resolved_policy_t = std::function<bool(const mt_name_t&)>;
bool isSrcNotEditedAfterCompilation(const mt_name_t &mt,
                                 bool useOutputDirAsCompiledMTDir);
static is_resolved_policy_t defaultPolicy = [](const mt_name_t &mt) {
	return isSrcNotEditedAfterCompilation(mt, false);
};

class DependencyCollector {
  impl::Trie<std::set<mt_name_t>> deps_;
  impl::Trie<bool> isresolved_;
  impl::Trie<std::string> mtNameFileNameMap_;
	impl::Trie<bool> isbuilded_;

public:
  DependencyCollector() = default;

  std::set<mt_name_t> getDepsFor(const mt_name_t &mt) {
    const auto &value = deps_.find(mt);
    if (!value.has_value())
      return {};
    return value.value();
  }

  std::string getFileNameFor(const mt_name_t &mt) {
    const auto &value = mtNameFileNameMap_.find(mt);
    if (!value.has_value())
      ThrowSourceNotFoundError error(mt);
    return value.value();
  }

  /**
   * @brief collects depedencies from program
   *
   * @param path -- path to program
   *
   * @return program main mt name
   */
  mt_name_t collect(const std::filesystem::path &path,
                    is_resolved_policy_t policy = defaultPolicy) {
		logger::info() << "Parsing: " << path << '\n'; 
    auto ast = toAST(path);
    const auto &mtName = ast.getTreeName();

    mtNameFileNameMap_.add(mtName, path);
    const auto &libs = ast.libs;

    std::set<mt_name_t> deps;
    for (const auto &lib : libs)
      deps.insert(lib);
    deps_.add(mtName, deps);
    isresolved_.add(mtName, policy(mtName));
		logger::log() << "Parsing: OK\n";

    return mtName;
  }

  bool resolve(const mt_name_t &mt, bool useLibDirAsOutputDir = true) {
		if(isbuilded_.contains(mt)) 
			return false;
		
		bool childsChanged = false;
    // resolving depedencies first
    const auto &deps = getDepsFor(mt);
    for (const auto &dep : deps) {
      childsChanged = resolve(dep) || childsChanged;
		}

    if (isresolved_.find(mt).value() && !childsChanged && !forceRecompile) {
      logger::info() << "MT [" << mt << "] is up to date. Skipping";
			childsChanged = false;
    } else {
			std::string fileName = getFileNameFor(mt);
			logger::info() << "MT [" << mt << "] was changed after compilation\n\t"
										 << "Following file will be recompiled: " << fileName;
			compileAndSaveProgram(fileName,
										globals::libDir,
										(useLibDirAsOutputDir ? globals::libDir : globals::outDir),
										globals::useBinaryFormat,
										globals::enableBreakpoints);
			*isresolved_.find(mt) = true;
			childsChanged = true;
		}
		isbuilded_.add(mt, true);
		return childsChanged;
  }
};

static DependencyCollector depsCollector;
namespace fs = std::filesystem;

void fillSources(const std::string &s) {
  auto paths = split(s, ',');
  for (const auto &path : paths) {
    sourcePaths.insert(strip(path));
  }

  for (const auto &path : sourcePaths) {
    logger::debug out;
    fs::directory_iterator it(path);
    for (auto &file : it) {
      if (file.path().extension() == ".ost") {
        try {
          depsCollector.collect(file.path());
        } catch (std::exception &e) {
          out << "Can't create AST from [" << file << "]. Skipping\n";
          continue;
        }
      }
    }
  }
}

void compileMainProgram(const std::string &fileName) {
  mt_name_t mt = depsCollector.collect(fileName, [](const mt_name_t & mt) {return isSrcNotEditedAfterCompilation(mt, true);});
  depsCollector.resolve(mt, false);
}

bool isSrcNotEditedAfterCompilation(const mt_name_t &mt,
                                 bool useOutputDirAsCompiledMTDir = false) {
  fs::path compiledMtPath;
  if (useOutputDirAsCompiledMTDir)
    compiledMtPath = globals::outDir;
  else
    compiledMtPath = globals::libDir;
  compiledMtPath.append(mt + ".tu4");
  if (!fs::exists(compiledMtPath))
    return false;

  std::string srcPath = depsCollector.getFileNameFor(mt);
  fs::file_time_type srcModifyTime = fs::last_write_time(srcPath),
                     compiledModifyTime = fs::last_write_time(compiledMtPath);
  logger::debug() << "mt: " << mt << " src modify time " << srcModifyTime
                  << " compiled modify time " << compiledModifyTime;
  return srcModifyTime < compiledModifyTime;
};
