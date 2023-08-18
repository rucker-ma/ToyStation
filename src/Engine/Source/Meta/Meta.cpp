#include "Meta.h"
//#include "Gen.h"
namespace toystation::reflect {
MetaClass& GetByName(const std::string& name) {
  return *Registry::Instance().Find(name);
}
}  // namespace toystation::reflect