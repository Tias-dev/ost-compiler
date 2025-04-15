#ifndef TRIE_HPP_
#define TRIE_HPP_

#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
namespace impl {
template <typename TData> class IActivePoint {
	public:
  virtual bool isTerm() const = 0;
  virtual bool canGoTo(char c) const = 0;
  virtual bool isLeaf() const = 0;
  virtual IActivePoint &goTo(char c) = 0;
  virtual TData get() = 0;
};

template <typename TData> class Trie {
  class NodeIterator;
  struct Node {
    std::optional<TData> data_ = std::nullopt;
    std::map<char, Node *> tr_ = {};
		Node * parent_ = nullptr;

    friend NodeIterator;

    Node() = default;
    Node(TData data) : data_(data) {}
		Node(Node * parent) : parent_(parent) {}
		Node(TData data, Node * parent) : data_(data), parent_(parent) {}

    bool isTerm() const;
    bool isLeaf() const;
    const TData &data() const;
		std::optional<const TData &> data_opt() const;
  };

  class NodeIterator : public IActivePoint<TData> {
    Node *node_;

	public:
    NodeIterator(Node *node) : node_(node) {
      if (!node) {
        throw std::invalid_argument("Node iterator must point on real node");
      }
    }

    bool isTerm() const override { return node_->isTerm(); }
    bool isLeaf() const override { return node_->isLeaf(); }
    bool canGoTo(char c) const override {
      return node_->tr_.find(c) != std::end(node_->tr_);
    }
    IActivePoint<TData>& goTo(char c) override {
      if (!canGoTo(c))
        throw std::logic_error(
            "Trying to go to next node but given translation doesn't exists");

      node_ = node_->tr_[c];
      return *this;
    }

		void addTranslation(char c) {
			if(canGoTo(c)) 
				throw std::invalid_argument("Given translation already exists");
			
			node_->tr_[c] = new Node(node_);
		}
		void setData(TData data) {
			node_->data_ = data;
		}
  };
  Node *root_ = new Node;
	using point_type_ = std::shared_ptr<NodeIterator>;
  point_type_ begin_() {
    return std::make_shared(NodeIterator(root_));
  }

public:
  Trie() {};

  void add(const std::string &s, TData data);
  std::optional<const TData &> find(const std::string &s) const;

	using point_type = std::shared_ptr<IActivePoint<TData>>;
  point_type begin() {
    return begin_();
  }
};

template <typename TData> bool Trie<TData>::Node::isTerm() const {
  return data_.has_value();
}

template <typename TData> bool Trie<TData>::Node::isLeaf() const {
  return tr_.size() == 0;
}

template <typename TData> 
std::optional<const TData &> Trie<TData>::Node::data_opt() const {
	return data_;
}

template <typename TData> const TData &Trie<TData>::Node::data() const {
  if (isTerm())
    return data_.value();
  throw std::runtime_error("Trying access to data of non term trie node");
}

template <typename TData>
void Trie<TData>::add(const std::string &s, TData data) {
	point_type_ activePoint = begin_();
	for(auto& c : s) {
		if(!activePoint->canGoTo(c)) 
			activePoint->addTranslation(c);
		activePoint->goTo(c);
	}
	activePoint->setData(data);
}

template <typename TData>
std::optional<const TData &> Trie<TData>::find(const std::string &s) const {
	point_type_ activePoint = begin_();
	for(auto& c : s) {
		if(!activePoint->canGoTo(c)) 
			return std::nullopt;
		activePoint->goTo(c);
	}
	if(!activePoint->isTerm()) 
		return std::nullopt;

	return activePoint->node_->data_opt();
}
} // namespace impl

#endif // !TRIE_HPP_
