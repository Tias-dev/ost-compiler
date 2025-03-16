#include "keywords.hpp"
#include "trie.hpp"

const static std::vector<std::pair<const std::string, KeywordType>> kwData{
    {"BEGIN", KeywordType::BEGIN}, {"END", KeywordType::END},
    {"DO", KeywordType::DO},       {"OD", KeywordType::OD},
    {"IF", KeywordType::IF},       {"FI", KeywordType::FI},
    {"MT", KeywordType::MT},       {"ALPHABET", KeywordType::ALPHABET},
    {"LIB", KeywordType::LIB},};

const static Trie<KeywordType> kwTrie(kwData);

KeywordType getKeywordType(const std::string &s) {
  auto value = kwTrie[s];
  if (value.has_value())
    return *value;
  return KeywordType::None;
}

bool isKeyword(const std::string &s) { return kwTrie[s].has_value(); }

std::string to_string(KeywordType type) {
  if (type == KeywordType::None)
    return "None";

  for (auto &kw : kwData) {
    if (kw.second == type) {
      return kw.first;
    }
  }

  return "Undefined";
}
