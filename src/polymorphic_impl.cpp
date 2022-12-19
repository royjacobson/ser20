#include "ser20/types/polymorphic.hpp"

namespace ser20::detail {

PolymorphicCaster::PolymorphicCaster() = default;
PolymorphicCaster::PolymorphicCaster(const PolymorphicCaster&) = default;
PolymorphicCaster&
PolymorphicCaster::operator=(const PolymorphicCaster&) = default;
PolymorphicCaster::PolymorphicCaster(PolymorphicCaster&&) noexcept {}
PolymorphicCaster& PolymorphicCaster::operator=(PolymorphicCaster&&) noexcept {
  return *this;
}
PolymorphicCaster::~PolymorphicCaster() noexcept = default;

std::pair<bool, std::vector<PolymorphicCaster const*> const&>
PolymorphicCasters::lookup_if_exists(std::type_index const& baseIndex,
                                     std::type_index const& derivedIndex) {
  // First phase of lookup - match base type index
  const auto& baseMap = StaticObject<PolymorphicCasters>::getInstance().map;
  auto baseIter = baseMap.find(baseIndex);
  if (baseIter == baseMap.end())
    return {false, {}};

  // Second phase - find a match from base to derived
  const auto& derivedMap = baseIter->second;
  auto derivedIter = derivedMap.find(derivedIndex);
  if (derivedIter == derivedMap.end())
    return {false, {}};

  return {true, derivedIter->second};
}

const std::vector<const PolymorphicCaster*>*
PolymorphicCasters::lookup(const std::type_index& baseIndex,
                           const std::type_index& derivedIndex) {
  // First phase of lookup - match base type index
  const auto& baseMap = StaticObject<PolymorphicCasters>::getInstance().map;
  auto baseIter = baseMap.find(baseIndex);
  if (baseIter == baseMap.end())
    return nullptr;

  // Second phase - find a match from base to derived
  const auto& derivedMap = baseIter->second;
  auto derivedIter = derivedMap.find(derivedIndex);
  if (derivedIter == derivedMap.end())
    return nullptr;

  return &(derivedIter->second);
}

void register_virtual_caster(const std::type_index baseKey,
                             const std::type_index derivedKey,
                             PolymorphicCaster* caster) {

  const auto lock = StaticObject<PolymorphicCasters>::lock();
  auto& baseMap = StaticObject<PolymorphicCasters>::getInstance().map;

  {
    auto& derivedMap =
        baseMap.insert({baseKey, PolymorphicCasters::DerivedCasterMap{}})
            .first->second;
    auto& derivedVec = derivedMap.insert({derivedKey, {}}).first->second;
    derivedVec.push_back(caster);
  }

  // Insert reverse relation Derived->Base
  auto& reverseMap = StaticObject<PolymorphicCasters>::getInstance().reverseMap;
  reverseMap.emplace(derivedKey, baseKey);

  // Find all chainable unregistered relations
  /* The strategy here is to process only the nodes in the class hierarchy
     graph that have been affected by the new insertion. The aglorithm
     iteratively processes a node an ensures that it is updated with all new
     shortest length paths. It then rocesses the parents of the active node,
     with the knowledge that all children have already been processed.

     Note that for the following, we'll use the nomenclature of parent and
     child to not confuse with the inserted base derived relationship */
  {
    // Checks whether there is a path from parent->child and returns a <dist,
    // path> pair dist is set to MAX if the path does not exist
    auto checkRelation = [](std::type_index const& parentInfo,
                            std::type_index const& childInfo)
        -> std::pair<size_t, std::vector<PolymorphicCaster const*> const&> {
      auto result = PolymorphicCasters::lookup_if_exists(parentInfo, childInfo);
      if (result.first) {
        auto const& path = result.second;
        return {path.size(), path};
      } else
        return {(std::numeric_limits<size_t>::max)(), {}};
    };

    std::stack<std::type_index>
        parentStack; // Holds the parent nodes to be processed
    std::vector<std::type_index>
        dirtySet; // Marks child nodes that have been changed
    std::unordered_set<std::type_index>
        processedParents; // Marks parent nodes that have been processed

    // Checks if a child has been marked dirty
    auto isDirty = [&](std::type_index const& c) {
      auto const dirtySetSize = dirtySet.size();
      for (size_t i = 0; i < dirtySetSize; ++i)
        if (dirtySet[i] == c)
          return true;

      return false;
    };

    // Begin processing the base key and mark derived as dirty
    parentStack.push(baseKey);
    dirtySet.emplace_back(derivedKey);

    while (!parentStack.empty()) {
      using Relations = std::unordered_multimap<
          std::type_index,
          std::pair<std::type_index, std::vector<PolymorphicCaster const*>>>;
      Relations unregisteredRelations; // Defer insertions until after main loop
                                       // to prevent iterator invalidation

      const auto parent = parentStack.top();
      parentStack.pop();

      // Update paths to all children marked dirty
      for (auto const& childPair : baseMap[parent]) {
        const auto child = childPair.first;
        if (isDirty(child) && baseMap.count(child)) {
          auto parentChildPath = checkRelation(parent, child);

          // Search all paths from the child to its own children (finalChild),
          // looking for a shorter parth from parent to finalChild
          for (auto const& finalChildPair : baseMap[child]) {
            const auto finalChild = finalChildPair.first;

            auto parentFinalChildPath = checkRelation(parent, finalChild);
            auto childFinalChildPath = checkRelation(child, finalChild);

            const size_t newLength = 1u + parentChildPath.first;

            if (newLength < parentFinalChildPath.first) {
              std::vector<PolymorphicCaster const*> path =
                  parentChildPath.second;
              path.insert(path.end(), childFinalChildPath.second.begin(),
                          childFinalChildPath.second.end());

              // Check to see if we have a previous uncommitted path in
              // unregisteredRelations that is shorter. If so, ignore this
              // path
              auto hintRange = unregisteredRelations.equal_range(parent);
              auto hint = hintRange.first;
              for (; hint != hintRange.second; ++hint)
                if (hint->second.first == finalChild)
                  break;

              const bool uncommittedExists =
                  hint != unregisteredRelations.end();
              if (uncommittedExists &&
                  (hint->second.second.size() <= newLength))
                continue;

              auto newPath = std::pair<std::type_index,
                                       std::vector<PolymorphicCaster const*>>{
                  finalChild, std::move(path)};

              // Insert the new path if it doesn't exist, otherwise this will
              // just lookup where to do the replacement
              auto old =
                  unregisteredRelations.emplace_hint(hint, parent, newPath);

              // If there was an uncommitted path, we need to perform a
              // replacement
              if (uncommittedExists)
                old->second = newPath;
            }
          } // end loop over child's children
        }   // end if dirty and child has children
      }     // end loop over children

      // Insert chained relations
      for (auto const& it : unregisteredRelations) {
        auto& derivedMap = baseMap.find(it.first)->second;
        derivedMap[it.second.first] = it.second.second;
        reverseMap.emplace(it.second.first, it.first);
      }

      // Mark current parent as modified
      dirtySet.emplace_back(parent);

      // Insert all parents of the current parent node that haven't yet been
      // processed
      auto parentRange = reverseMap.equal_range(parent);
      for (auto pIter = parentRange.first; pIter != parentRange.second;
           ++pIter) {
        const auto pParent = pIter->second;
        if (!processedParents.count(pParent)) {
          parentStack.push(pParent);
          processedParents.insert(pParent);
        }
      }
    } // end loop over parent stack
  }   // end chainable relations
}

} // namespace ser20::detail
