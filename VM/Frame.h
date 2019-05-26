#pragma once

#include <Object/Object.h>

namespace monkey::vm {

struct Frame {
  Frame();
  template <typename T>
  Frame(T &&Cl, int BasePointer)
      : Cl(std::forward<T>(Cl)), IP(-1), BasePointer(BasePointer) {}

  code::Instructions &instructions();

  std::shared_ptr<object::Object> Cl;
  int IP;
  int BasePointer;
};

} // namespace monkey::vm
