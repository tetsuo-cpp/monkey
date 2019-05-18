#pragma once

#include <Object/Object.h>

namespace monkey::vm {

struct Frame {
  Frame();
  template <typename T> Frame(T &&Fn) : Fn(std::forward<T>(Fn)), IP(-1) {}

  code::Instructions &instructions();

  std::shared_ptr<object::Object> Fn;
  int IP;
};

} // namespace monkey::vm
