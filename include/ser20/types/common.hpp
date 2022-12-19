/*! \file common.hpp
    \brief Support common types - always included automatically
    \ingroup OtherTypes */
/*
  Copyright (c) 2014, Randolph Voorhies, Shane Grant
  Copyright (c) 2022, Roy Jacobson
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of the copyright holder nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef SER20_TYPES_COMMON_HPP_
#define SER20_TYPES_COMMON_HPP_

#include "ser20/ser20.hpp"

namespace ser20 {
namespace common_detail {

namespace detail {
//! Gets the underlying type of an enum
/*! @internal */
template <class T, bool IsEnum>
struct enum_underlying_type : std::false_type {};

//! Gets the underlying type of an enum
/*! Specialization for when we actually have an enum
    @internal */
template <class T> struct enum_underlying_type<T, true> {
  using type SER20_NODEBUG = std::underlying_type_t<T>;
};
} // namespace detail

//! Checks if a type is an enum
/*! This is needed over simply calling std::is_enum because the type
    traits checking at compile time will attempt to call something like
    load_minimal with a special NoConvertRef struct that wraps up the true type.

    This will strip away any of that and also expose the true underlying type.
    @internal */
template <class T> class is_enum {
private:
  using DecayedT = std::decay_t<T>;
  using StrippedT = typename ::ser20::traits::strip_minimal<DecayedT>::type;

public:
  static const bool value = std::is_enum_v<StrippedT>;
  using type SER20_NODEBUG = StrippedT;
  using base_type SER20_NODEBUG =
      typename detail::enum_underlying_type<StrippedT, value>::type;
};
} // namespace common_detail

//! Saving for enum types
template <class Archive, class T>
inline typename common_detail::is_enum<T>::base_type
SER20_SAVE_MINIMAL_FUNCTION_NAME(Archive const&, T const& t)
  requires(common_detail::is_enum<T>::value)
{
  return static_cast<typename common_detail::is_enum<T>::base_type>(t);
}

//! Loading for enum types
template <class Archive, class T>
void SER20_LOAD_MINIMAL_FUNCTION_NAME(
    Archive const&, T&& t,
    typename common_detail::is_enum<T>::base_type const& value)
  requires(common_detail::is_enum<T>::value)
{
  t = reinterpret_cast<typename common_detail::is_enum<T>::type const&>(value);
}

//! Serialization for raw pointers
/*! This exists only to throw a static_assert to let users know we don't support
 * raw pointers. */
template <class Archive, class T>
inline void SER20_SERIALIZE_FUNCTION_NAME(Archive&, T*&) {
  static_assert(ser20::traits::detail::delay_static_assert<T>,
                "Ser20 does not support serializing raw pointers - please use "
                "a smart pointer");
}

//! Serialization for C style arrays
template <class Archive, class T>
inline void SER20_SERIALIZE_FUNCTION_NAME(Archive& ar, T& array)
  requires(std::is_array_v<T>)
{
  constexpr bool supportsBinaryData =
      std::is_arithmetic_v<std::remove_all_extents_t<T>> &&
      (traits::is_output_serializable_v<BinaryData<T>, Archive> ||
       traits::is_input_serializable_v<BinaryData<T>, Archive>);

  if constexpr (supportsBinaryData) {
    ar(binary_data(array, sizeof(array)));
  } else {
    for (auto& i : array)
      ar(i);
  }
}
} // namespace ser20

#endif // SER20_TYPES_COMMON_HPP_
