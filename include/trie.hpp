#ifndef TRIE_HPP_
#define TRIE_HPP_

#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
namespace impl {
template <typename TData> class IActivePoint {
protected:
  virtual const void *dataPtr() = 0;

public:
  virtual bool isTerm() const = 0;
  virtual bool canGoTo(char c) const = 0;
  virtual bool isLeaf() const = 0;
  virtual IActivePoint &goTo(char c) = 0;
  virtual void goBack() = 0;
  virtual const TData & get() const = 0;

  virtual bool operator==(IActivePoint<TData> &rhs) {
    return this->dataPtr() == rhs.dataPtr();
  }
  virtual bool operator!=(IActivePoint<TData> &rhs) { return !(*this == rhs); }
};

template <typename TData> class Trie {
  class NodeIterator;
  struct Node {
    std::optional<TData> data_ = std::nullopt;
    std::map<char, Node *> tr_ = {};
    Node *parent_ = nullptr;

    friend NodeIterator;

    Node() = default;
    Node(TData data) : data_(data) {}
    Node(Node *parent) : parent_(parent) {}
    Node(TData data, Node *parent) : data_(data), parent_(parent) {}

    bool isTerm() const;
    bool isLeaf() const;
    const TData &data() const;
    std::optional<TData> & data_opt();
  };

  class NodeIterator : public IActivePoint<TData> {
    Node *node_;
    const void *dataPtr() override { return node_; }

  public:
    NodeIterator(Node *node) : node_(node) {
      if (!node) {
        throw std::invalid_argument("Node iterator must point on real node");
      }
    }

		const TData &get() const override {
			return node_->data();
		}

    bool isTerm() const override { return node_->isTerm(); }
    bool isLeaf() const override { return node_->isLeaf(); }
    bool canGoTo(char c) const override {
      return node_->tr_.find(c) != std::end(node_->tr_);
    }
    IActivePoint<TData> &goTo(char c) override {
      if (!canGoTo(c))
        throw std::logic_error(
            "Trying to go to next node but given translation doesn't exists");

      node_ = node_->tr_[c];
      return *this;
    }

    void goBack() override {
      if (node_->parent_)
        node_ = node_->parent_;
    }

    void addTranslation(char c) {
      if (canGoTo(c))
        throw std::invalid_argument("Given translation already exists");

      node_->tr_[c] = new Node(node_);
    }
    void setData(TData data) { node_->data_ = data; }

		Node * node() {return node_;}
  };

  Node *root_ = new Node;

  using point_type_ = std::shared_ptr<NodeIterator>;
  point_type_ begin_() const { return std::make_shared<NodeIterator>(root_); }

	class Bimap {
		std::map<std::string, TData> toData_;
		std::map<TData, std::string> toString_;
		friend Trie<TData>;

		void add(std::string s, TData data) {
			toData_[s] = data;
			toString_[data] = s;
		}
	public:
		TData toData(const std::string & s) const { return toData_[s]; }
		const std::string & toString(const TData & data) const { return toString_.at(data); }
	};

	void bimap_(Node *, Bimap &, std::string &) const;
public:
  Trie() {};

  void add(const std::string &s, TData data);
  std::optional<TData>& find(const std::string &s);
	bool contains(const std::string &s) const {
		return find(s).has_value();
	}

  using point_type = std::shared_ptr<IActivePoint<TData>>;
	using bimap_type = Bimap;

  /**
   * @brief Create single point i.e. shifting point forward and backward not
   * creating new point, but shift current
   *
   * @return point at root of tree
   */
  point_type begin() const { return begin_(); }
	bimap_type bimap() const;
};

template <typename TData> bool Trie<TData>::Node::isTerm() const {
  return data_.has_value();
}

template <typename TData> bool Trie<TData>::Node::isLeaf() const {
  return tr_.size() == 0;
}

template <typename TData>
std::optional<TData>& Trie<TData>::Node::data_opt() {
  return data_;
}

template <typename TData> const TData &Trie<TData>::Node::data() const {
  if (!isTerm())
		throw std::logic_error("Trying get to data from node which have no data");
	return data_.value();
}

template <typename TData>
void Trie<TData>::add(const std::string &s, TData data) {
  point_type_ activePoint = begin_();
  for (auto &c : s) {
    if (!activePoint->canGoTo(c))
      activePoint->addTranslation(c);
    activePoint->goTo(c);
  }
  activePoint->setData(data);
}

template <typename TData>
std::optional<TData>& Trie<TData>::find(const std::string &s) {
	static std::optional<TData> nullopt = std::nullopt;
  point_type_ activePoint = begin_();
  for (auto &c : s) {
    if (!activePoint->canGoTo(c))
      return nullopt;
    activePoint->goTo(c);
  }
  if (!activePoint->isTerm())
    return nullopt;

  return activePoint->node()->data_opt();
}

template <typename TData>
typename Trie<TData>::bimap_type
Trie<TData>::bimap() const {
	Bimap bimap;
	std::string buffer;
	bimap_(root_, bimap, buffer);
	return bimap;
}

template <typename TData>
void
Trie<TData>::bimap_(Node * node, Bimap & bimap, std::string & buffer) const {
	if(node->isTerm()) 
		bimap.add(buffer, node->data());
	
	for(auto& [c, child] : node->tr_) {
		buffer.push_back(c);
		bimap_(child, bimap, buffer);
		buffer.pop_back();
	}
}

} // namespace impl

#endif // !TRIE_HPP_
