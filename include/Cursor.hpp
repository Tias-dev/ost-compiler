#ifndef CURSOR_HPP_
#define CURSOR_HPP_

// Stores some data about
// current cursor's position

#include "CharStream.hpp"
#include "event.hpp"
#include <cstddef>
#include <memory>
#include <stack>
class ICursor {
public:
  /**
   * @brief Get line the cursor placed at
   *
   * @return index of the line
   */
  virtual size_t line() = 0;

  /**
   * @brief Get column the cursor placed at
   *
   * @return index of the column
   */
  virtual size_t column() = 0;

  /**
   * @brief Get cursor position index from the begining of stream
   *
   * @return The index
   */
  virtual size_t index() = 0;
};

/**
 * @brief Event based cursor
 */
class EventCursor : public ICursor {
private:
  std::shared_ptr<ICharStream> stream_;

  size_t line_ = 0;
  size_t column_ = 0;
  size_t index_ = 0;
  std::stack<size_t> newLines_;

  struct EventHandler : public IEventHandler<char> {
    EventCursor *cursor_ = nullptr;
    void (*fun_)(EventCursor *, char) = nullptr;

    EventHandler(EventCursor *cursor, void (*fun)(EventCursor *, char))
        : fun_(fun), cursor_(cursor) {}

    EventHandler() = default;

    void operator()(char letter) { fun_(cursor_, letter); }
  };

  EventHandler onGetChar_;
  EventHandler onReturnChar_;

public:
  EventCursor(std::shared_ptr<ICharStream> stream);
  virtual ~EventCursor();

  size_t line() override { return line_; }
  size_t column() override { return column_; }
  size_t index() override { return index_; }
};

#endif // !CURSOR_HPP_
