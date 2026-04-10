#ifndef NVBOARD_INCLUDE_NVBOARD_NVBOARD_H_
#define NVBOARD_INCLUDE_NVBOARD_NVBOARD_H_

#include <cstdint>
#include <memory>

#include "absl/types/span.h"

namespace nvboard {

class BoardImpl;

class Board {
 public:
  static std::unique_ptr<Board> Create(int vga_clk_cycle = 1);
  ~Board();

  Board(const Board&) = delete;
  Board& operator=(const Board&) = delete;

  void BindPin(void* signal, absl::Span<const int> pins);
  void Update();

  class Kbd;
  class Vga;
  class Uart;

  Kbd& kbd();
  Vga& vga();
  Uart& uart();

  uint16_t SwRead();
  uint8_t BtnRead();
  void LedWrite(uint16_t val);
  void Seg7Write(int idx, uint8_t segments);

 private:
  Board();
  std::unique_ptr<BoardImpl> impl_;
};

class Board::Kbd {
 public:
  bool Available();
  uint8_t Dequeue();

 private:
  friend class BoardImpl;
  BoardImpl* impl_;
  explicit Kbd(BoardImpl* impl) : impl_(impl) {}
};

class Board::Vga {
 public:
  void SetFramebuffer(uint32_t* fb, int w, int h);
  void Sync();

 private:
  friend class BoardImpl;
  BoardImpl* impl_;
  explicit Vga(BoardImpl* impl) : impl_(impl) {}
};

class Board::Uart {
 public:
  void Putchar(uint8_t ch);
  bool Available();
  uint8_t Getchar();

 private:
  friend class BoardImpl;
  BoardImpl* impl_;
  explicit Uart(BoardImpl* impl) : impl_(impl) {}
};

}  // namespace nvboard

#endif  // NVBOARD_INCLUDE_NVBOARD_NVBOARD_H_
