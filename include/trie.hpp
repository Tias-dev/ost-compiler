#ifndef TRIE_HPP_
#define TRIE_HPP_

#include <map>
#include <optional>
#include <string>
#include <vector>

#define in(container, item) (container.find(item) != container.end())

template <typename T> class Trie {
public:
  Trie();
  Trie(const std::vector<std::pair<const std::string, T>> &data);

  void insert(const std::string &, T);
  void remove(std::string &);

  bool find(const std::string &);
  std::optional<T> operator[](const std::string &);
  const std::optional<T> operator[](const std::string &) const;

private:
  char terminator = '$';
  struct Node {
    std::map<char, Node *> translations;
    std::optional<T> data;
  };

  Node *root;
  // void impl_remove(std::string &, size_t curIndex, Node *currentNode);
};

template <typename T>
std::optional<T> Trie<T>::operator[](const std::string &s) {
  Node *currentNode = root;
  for (auto &c : s) {
    if (!in(currentNode->translations, c)) {
      return std::nullopt;
    }
    currentNode = currentNode->translations[c];
  }
  return currentNode->data;
}

template <typename T>
const std::optional<T> Trie<T>::operator[](const std::string &s) const {
  Node *currentNode = root;
  for (auto &c : s) {
    if (!in(currentNode->translations, c)) {
      return std::nullopt;
    }
    currentNode = currentNode->translations[c];
  }
  return currentNode->data;
}

template <typename T> inline Trie<T>::Trie() {
  root = new Node{std::map<char, Node *>{{terminator, nullptr}}};
}

template <typename T>
inline Trie<T>::Trie(const std::vector<std::pair<const std::string, T>> &data)
    : Trie() {
  for (auto &[s, value] : data) {
    insert(s, value);
  }
}

template <typename T>
inline void Trie<T>::insert(const std::string &s, T value) {
  if (find(s)) {
    return;
  }

  Node *currentNode = root;
  for (auto &c : s) {
    if (!in(currentNode->translations, c)) {
      currentNode->translations[c] = new Node;
    }
    currentNode = currentNode->translations[c];
  }
  currentNode->translations[terminator] = nullptr;
  currentNode->data = {value};
}

template <typename T> inline bool Trie<T>::find(const std::string &s) {
  Node *currentNode = root;
  for (auto &c : s) {
    if (!in(currentNode->translations, c)) {
      return false;
    }
    currentNode = currentNode->translations[c];
  }

  return in(currentNode->translations, terminator);
}

template <typename T> inline void Trie<T>::remove(std::string &s) {
  if (!find(s)) {
    return;
  }

  size_t curIndex = 0;
  Node *currentNode = root;

  std::vector<Node *> path;
  path.reserve(s.size());

  while (curIndex < s.size()) {
    path.emplace_back(currentNode);
    currentNode = currentNode->translations[s[curIndex]];
    ++curIndex;
  }

  currentNode->translations.erase(terminator);

  --curIndex;
  currentNode = path.back();
  path.pop_back();

  while (curIndex >= 0 && curIndex < s.size() &&
         currentNode->translations[s[curIndex]]->translations.size() == 0) {
    currentNode->translations.erase(s[curIndex]);
    currentNode = path.back();
    path.pop_back();
    --curIndex;
  }
}

#undef in

#endif // TRIE_HPP_
