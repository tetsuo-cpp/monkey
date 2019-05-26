#include "Frame.h"

namespace monkey::vm {

Frame::Frame() : Cl(nullptr), IP(-1), BasePointer(-1) {}

code::Instructions &Frame::instructions() {
  auto *ClObj = static_cast<object::Closure *>(Cl.get());
  return static_cast<object::CompiledFunction *>(ClObj->Fn.get())->Ins;
}

} // namespace monkey::vm
