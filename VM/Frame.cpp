#include "Frame.h"

namespace monkey::vm {

Frame::Frame() : Fn(nullptr), IP(-1), BasePointer(-1) {}

code::Instructions &Frame::instructions() {
  return static_cast<object::CompiledFunction *>(Fn.get())->Ins;
}

} // namespace monkey::vm
