#pragma once

#include <Object/Object.h>

namespace monkey::vm {

struct Frame {
  Frame();
  template <typename T>
  Frame(T &&Fn, int BasePointer)
      : Fn(std::forward<T>(Fn)), IP(-1), BasePointer(BasePointer) {}

  code::Instructions &instructions();

  std::shared_ptr<object::Object> Fn;
  int IP;
  int BasePointer;
};

} // namespace monkey::vm
