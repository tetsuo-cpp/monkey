#include "BuiltIns.h"

#include <cassert>
#include <stdarg.h>

namespace monkey::object {

// Null global should probably go in here. Instead, we check for nullptr in the
// evaluator and vm and then return a Null object. So it's just a wasted
// construction of a shared_ptr.
const std::vector<std::pair<std::string, std::shared_ptr<BuiltIn>>> BUILTINS = {
    {"len",
     std::make_shared<BuiltIn>([](const std::vector<std::shared_ptr<Object>>
                                      &Args) -> std::shared_ptr<Object> {
       if (Args.size() != 1)
         return newError("wrong number of arguments. got=%d, want=1",
                         Args.size());

       const auto *StringObj = objCast<const String *>(Args.front().get());
       if (StringObj)
         return makeInteger(StringObj->Value.size());

       const auto *ArrayObj = objCast<const Array *>(Args.front().get());
       if (ArrayObj)
         return makeInteger(ArrayObj->Elements.size());

       return newError("argument to \"len\" not supported, got %s",
                       objTypeToString(Args.front()->type()));
     })},
    {"first",
     std::make_shared<BuiltIn>([](const std::vector<std::shared_ptr<Object>>
                                      &Args) -> std::shared_ptr<Object> {
       if (Args.size() != 1)
         return newError("wrong number of arguments. got=%d, want=1",
                         Args.size());

       if (Args.front()->type() != ObjectType::ARRAY_OBJ)
         return newError("argument to \"first\" must be ARRAY, got %s",
                         objTypeToString(Args.front()->type()));

       const auto *ArrayObj = objCast<const Array *>(Args.front().get());
       assert(ArrayObj);
       if (!ArrayObj->Elements.empty())
         return ArrayObj->Elements.front();

       return NULL_GLOBAL;
     })},
    {"last",
     std::make_shared<BuiltIn>([](const std::vector<std::shared_ptr<Object>>
                                      &Args) -> std::shared_ptr<Object> {
       if (Args.size() != 1)
         return newError("wrong number of arguments. got=%d, want=1",
                         Args.size());

       if (Args.front()->type() != ObjectType::ARRAY_OBJ)
         return newError("argument to \"last\" must be ARRAY, got %s",
                         objTypeToString(Args.front()->type()));

       const auto *ArrayObj = objCast<const Array *>(Args.front().get());
       assert(ArrayObj);
       if (!ArrayObj->Elements.empty())
         return ArrayObj->Elements.back();

       return NULL_GLOBAL;
     })},
    {"rest",
     std::make_shared<BuiltIn>([](const std::vector<std::shared_ptr<Object>>
                                      &Args) -> std::shared_ptr<Object> {
       if (Args.size() != 1)
         return newError("wrong number of arguments. got=%d, want=1",
                         Args.size());

       if (Args.front()->type() != ObjectType::ARRAY_OBJ)
         return newError("argument to \"rest\" must be ARRAY, got %s",
                         objTypeToString(Args.front()->type()));

       const auto *ArrayObj = objCast<const Array *>(Args.front().get());
       assert(ArrayObj);
       if (!ArrayObj->Elements.empty()) {
         std::vector<std::shared_ptr<Object>> Rest;
         std::copy(ArrayObj->Elements.begin() + 1, ArrayObj->Elements.end(),
                   std::back_inserter(Rest));
         return std::make_shared<Array>(std::move(Rest));
       }

       return NULL_GLOBAL;
     })},
    {"push",
     std::make_shared<BuiltIn>([](const std::vector<std::shared_ptr<Object>>
                                      &Args) -> std::shared_ptr<Object> {
       if (Args.size() != 2)
         return newError("wrong number of arguments. got=%d, want=2",
                         Args.size());

       if (Args.front()->type() != ObjectType::ARRAY_OBJ)
         return newError("argument to \"push\" must be ARRAY, got %s",
                         objTypeToString(Args.front()->type()));

       const auto *ArrayObj = objCast<const Array *>(Args.front().get());
       assert(ArrayObj);
       auto Pushed = ArrayObj->Elements;
       Pushed.push_back(Args.at(1));
       return makeArray(std::move(Pushed));
     })},
    {"puts", std::make_shared<BuiltIn>(
                 [](const std::vector<std::shared_ptr<Object>> &Args)
                     -> std::shared_ptr<Object> {
                   for (const auto &Arg : Args)
                     printf("%s\n", Arg->inspect().c_str());

                   return NULL_GLOBAL;
                 })}};

std::shared_ptr<Error> newError(const char *Format, ...) {
#define ERROR_SIZE 1024
  char ErrorBuf[ERROR_SIZE];
  va_list ArgList;
  va_start(ArgList, Format);
  vsnprintf(ErrorBuf, ERROR_SIZE, Format, ArgList);
  va_end(ArgList);
  return std::make_shared<Error>(ErrorBuf);
}

std::shared_ptr<BuiltIn> getBuiltInByName(const std::string &Name) {
  const auto Iter = std::find_if(
      BUILTINS.begin(), BUILTINS.end(),
      [&Name](const std::pair<std::string, std::shared_ptr<BuiltIn>> &P) {
        return P.first == Name;
      });

  if (Iter == BUILTINS.end())
    return nullptr;

  return Iter->second;
}

} // namespace monkey::object
