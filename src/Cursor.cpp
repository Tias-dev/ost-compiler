#include "Cursor.hpp"
#include "CharStream.hpp"
#include <memory>

EventCursor::EventCursor(std::shared_ptr<ICharStream> stream) : stream_(stream) {
  onGetChar_ = EventHandler(this, [](EventCursor *cursor, char letter) {
    if (letter == '\n') {
      cursor->newLines_.push(cursor->index_);
      ++cursor->line_;
      cursor->column_ = 0;
    } else
      ++cursor->column_;
    ++cursor->index_;
  });

  onReturnChar_ = EventHandler(this, [](EventCursor *cursor, char letter) {
    if (letter == '\n') {
      size_t oldNewLine = cursor->newLines_.top();
      cursor->newLines_.pop();
      size_t curNewLine = cursor->newLines_.top();

      --cursor->line_;
      cursor->column_ = oldNewLine - curNewLine;
    } else
      --cursor->column_;
    --cursor->index_;
  });

  stream->onGetChar += onGetChar_;
  stream->onReturnChar += onReturnChar_;
}

EventCursor::~EventCursor() {
  stream_->onGetChar -= onGetChar_;
  stream_->onReturnChar -= onReturnChar_;
}
