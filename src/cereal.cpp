#include "cereal/cereal.hpp"

namespace cereal {

std::uint32_t OutputSharedPointerStorage::registerSharedPointer(
    const std::shared_ptr<const void>& sharedPointer) {
  void const* addr = sharedPointer.get();

  // Handle null pointers by just returning 0
  if (addr == 0)
    return 0;
  itsSharedPointerStorage.push_back(sharedPointer);

  auto id = itsSharedPointerMap.find(addr);
  if (id == itsSharedPointerMap.end()) {
    auto ptrId = itsCurrentPointerId++;
    itsSharedPointerMap.emplace(addr, ptrId);
    return ptrId | detail::msb_32bit; // mask MSB to be 1
  } else
    return id->second;
}

std::shared_ptr<void>
InputSharedPointerStorage::getSharedPointer(std::uint32_t const id) {
  if (id == 0)
    return std::shared_ptr<void>(nullptr);

  auto iter = itsSharedPointerMap.find(id);
  if (iter == itsSharedPointerMap.end())
    throw Exception("Error while trying to deserialize a smart pointer. "
                    "Could not find id " +
                    std::to_string(id));

  return iter->second;
}
void InputSharedPointerStorage::registerSharedPointer(
    std::uint32_t const id, std::shared_ptr<void> ptr) {
  std::uint32_t const stripped_id = id & ~detail::msb_32bit;
  itsSharedPointerMap[stripped_id] = ptr;
}

} // namespace cereal
