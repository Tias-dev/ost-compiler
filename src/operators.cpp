#include "operators.hpp"
#include "trie.hpp"
#include <string>
#include <utility>
#include <vector>

const static std::vector<std::pair<const std::string, OperatorType>>
    operatorData{
        {",", OperatorType::Comma},        {"=", OperatorType::Equal},
        {"!=", OperatorType::NotEqual},    {"?", OperatorType::Question},
        {"**", OperatorType::Pow},         {"(", OperatorType::OpenBracket},
        {")", OperatorType::CloseBracket}, {":", OperatorType::DoutbleDot},
        {";", OperatorType::Terminator}, {"a", OperatorType::SetLetter}, {"_", OperatorType::Lambda}};

const static Trie<OperatorType> operatorTrie{operatorData};

OperatorType getOperatorType(const std::string &s) {
  auto value = operatorTrie[s];
  if (value.has_value())
    return *value;
  return OperatorType::None;
}
bool isOperator(const std::string &s) { return operatorTrie[s].has_value(); }

std::string to_string(OperatorType type) {
  if (type == OperatorType::None)
    return "None";

  for (auto &op : operatorData) {
    if (op.second == type) {
      return op.first;
    }
  }

  return "Undefined";
}
