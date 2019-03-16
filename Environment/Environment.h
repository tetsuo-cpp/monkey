#pragma once

#include <Object/ObjectInterface.h>

#include <unordered_map>

namespace monkey::environment {

class Environment {
public:
  Environment();
  Environment(Environment *);
  virtual ~Environment() = default;

  std::shared_ptr<object::Object> get(const std::string &);
  void set(const std::string &, std::shared_ptr<object::Object>);

private:
  std::unordered_map<std::string, std::shared_ptr<object::Object>> Store;
  Environment *Outer;
};

} // namespace monkey::environment
