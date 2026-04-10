#ifndef NVBOARD_SRC_INTERNAL_COMPONENT_H_
#define NVBOARD_SRC_INTERNAL_COMPONENT_H_

#include <cstdint>
#include <vector>

#include <SDL.h>

namespace nvboard {

class BoardImpl;

enum class ComponentType {
  kButton = 1,
  kSwitch,
  kNaiveLed,
  kRgbLed,
  kSegs7,
  kVga,
  kKeyboard,
  kUart,
};

enum class LogicType { kComb = 1, kSeq = 2 };

class Component {
 public:
  Component(BoardImpl *board, int cnt, int init_val, ComponentType ct);
  virtual ~Component() = default;

  bool InRect(int x, int y) const;
  SDL_Renderer *GetRenderer() const;
  ComponentType GetComponentType() const;
  SDL_Rect *GetRect(int idx) const;
  SDL_Texture *GetTexture(int idx) const;
  int GetState() const;
  uint16_t GetPin(int idx = 0) const;

  void SetRect(SDL_Rect *rect, int val);
  void SetTexture(SDL_Texture *texture, int val);
  void SetState(int val);
  void AddPin(uint16_t pin);
  virtual void UpdateGui();
  virtual void UpdateState();
  void Remove();

 protected:
  BoardImpl *board_;

 private:
  ComponentType component_type_;
  std::vector<SDL_Rect *> rects_;
  std::vector<SDL_Texture *> textures_;
  int state_;
  std::vector<uint16_t> pins_;
};

class Segs7 : public Component {
 public:
  Segs7(BoardImpl *board, int cnt, int init_val, ComponentType ct,
        bool is_len8);
  void UpdateGui() override;
  void UpdateState() override;

 private:
  bool is_len8_;
};

void InitComponents(BoardImpl *impl);

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_COMPONENT_H_
