#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <iterator>
#include <list>
#include <utility>

// --------------------
// ---- INTERFASES ----
// --------------------

template <typename... Args> class IEventHandler;

template <typename... Args> class IEvent {
public:
  virtual void subscribe(IEventHandler<Args...> &handler) = 0;
  virtual void unsubscribe(IEventHandler<Args...> &handler) = 0;
  IEvent *operator+=(IEventHandler<Args...> &handler) {
    subscribe(handler);
    return this;
  }

  IEvent *operator-=(IEventHandler<Args...> &handler) {
    unsubscribe(handler);
    return this;
  }
};

template <typename... Args> class IEventHandler {
public:
  virtual void operator()(Args... args) = 0;
};

// --------------------
// --- BASE CLASSES ---
// --------------------

template <typename... Args> class Event : public IEvent<Args...> {
  std::list<IEventHandler<Args...> *> subscribers_;

public:
  virtual void invoke(Args... args) {
    for (auto &sub : subscribers_) {
      (*sub)(args...);
    }
  }

  virtual void subscribe(IEventHandler<Args...> &handler) {
    for (auto &sub : subscribers_) {
      if (sub == &handler) {
        return;
      }
    }
    subscribers_.push_back(&handler);
  }

  virtual void unsubscribe(IEventHandler<Args...> &handler) {
    auto it = std::begin(subscribers_);
    for (auto &sub : subscribers_) {
      if (sub == &handler) {
        subscribers_.erase(it);
        return;
      }
      ++it;
    }
  }
};

/**
 * @brief Handler that call lambda at notifying
 *
 * @tparam TLambda -- decltype(lambda) usual
 * @tparam ...Args -- args that handler(and lambda) will recieve
 */
template <typename TLambda, typename... Args>
class LambdaHandler : public IEventHandler<Args...> {
private:
  TLambda lambda_;

public:
  LambdaHandler(TLambda lambda) : lambda_(lambda) {}
  virtual void operator()(Args... args) override { lambda_(args...); }
};

#endif // !EVENT_HPP_
