#ifndef CURSOR_HPP_
#define CURSOR_HPP_

// Stores some data about
// current cursor's position

#include "CharStream.hpp"
#include "event.hpp"
#include <cstddef>
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
  virtual size_t curIndex() = 0;
};

class EventCursor : public ICursor {
private:
  size_t line_ = 0;
  size_t column_ = 0;
  size_t index_ = 0;

  LambdaHandler<void (*)(char), char> onGetCharHandler;

public:
  EventCursor(CharStream &stream);
};

#endif // !CURSOR_HPP_
