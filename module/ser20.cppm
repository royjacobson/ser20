module;

#include <atomic>
#include <bitset>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <forward_list>
#include <functional>
#include <inttypes.h>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>

#ifdef SER20_THREAD_SAFE
#if SER20_THREAD_SAFE
#include <mutex>
#endif
#endif

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

export module ser20;
export {
#include "ser20/access.hpp"
#include "ser20/ser20.hpp"
#include "ser20/macros.hpp"
#include "ser20/specialize.hpp"
#include "ser20/version.hpp"

#include "ser20/archives/adapters.hpp"
#include "ser20/archives/binary.hpp"
#include "ser20/archives/json.hpp"
#include "ser20/archives/portable_binary.hpp"
#include "ser20/archives/xml.hpp"

#include "ser20/types/array.hpp"
#include "ser20/types/atomic.hpp"
#include "ser20/types/base_class.hpp"
#include "ser20/types/bitset.hpp"
#include "ser20/types/boost_variant.hpp"
#include "ser20/types/chrono.hpp"
#include "ser20/types/common.hpp"
#include "ser20/types/complex.hpp"
#include "ser20/types/concepts/pair_associative_container.hpp"
#include "ser20/types/deque.hpp"
#include "ser20/types/forward_list.hpp"
#include "ser20/types/functional.hpp"
#include "ser20/types/list.hpp"
#include "ser20/types/map.hpp"
#include "ser20/types/memory.hpp"
#include "ser20/types/optional.hpp"
#include "ser20/types/polymorphic.hpp"
#include "ser20/types/queue.hpp"
#include "ser20/types/set.hpp"
#include "ser20/types/stack.hpp"
#include "ser20/types/string.hpp"
#include "ser20/types/tuple.hpp"
#include "ser20/types/unordered_map.hpp"
#include "ser20/types/unordered_set.hpp"
#include "ser20/types/utility.hpp"
#include "ser20/types/valarray.hpp"
#include "ser20/types/variant.hpp"
#include "ser20/types/vector.hpp"
}
