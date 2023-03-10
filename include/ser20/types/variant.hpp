/*! \file variant.hpp
    \brief Support for std::variant
    \ingroup STLSupport */
/*
  Copyright (c) 2014, 2017, Randolph Voorhies, Shane Grant, Juan Pedro
  Bolivar Puente.
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
#ifndef SER20_TYPES_STD_VARIANT_HPP_
#define SER20_TYPES_STD_VARIANT_HPP_

#include "ser20/ser20.hpp"
#include <cstdint>
#include <variant>

namespace ser20 {
namespace variant_detail {

//! @internal
template <int N, class Variant, class Archive>
inline void load_variant_at_index(Archive& ar, size_t target,
                                  Variant& variant) {
  if (target == N) {
    variant.template emplace<N>();
    ar(SER20_NVP_("data", std::get<N>(variant)));
  }
}

template <class Variant, class Archive, std::size_t... Is>
inline void load_variant(Archive& ar, size_t target, Variant& variant,
                         std::index_sequence<Is...>) {
  (load_variant_at_index<Is, Variant, Archive>(ar, target, variant), ...);
}

} // namespace variant_detail

//! Saving for std::variant
template <class Archive, typename VariantType1, typename... VariantTypes>
inline void SER20_SAVE_FUNCTION_NAME(
    Archive& ar, std::variant<VariantType1, VariantTypes...> const& variant) {
  std::int32_t index = static_cast<std::int32_t>(variant.index());
  ar(SER20_NVP_("index", index));
  std::visit([&ar](const auto& value) -> void { ar(SER20_NVP_("data", value)); },
             variant);
}

//! Loading for std::variant
template <class Archive, typename... VariantTypes>
inline void SER20_LOAD_FUNCTION_NAME(Archive& ar,
                                      std::variant<VariantTypes...>& variant) {
  using variant_t = std::variant<VariantTypes...>;

  std::int32_t index;
  ar(SER20_NVP_("index", index));
  if (index >= static_cast<std::int32_t>(std::variant_size_v<variant_t>))
    throw Exception("Invalid 'index' selector when deserializing std::variant");

  variant_detail::load_variant(
      ar, index, variant,
      std::make_index_sequence<std::variant_size_v<variant_t>>());
}

//! Serializing a std::monostate
template <class Archive>
void SER20_SERIALIZE_FUNCTION_NAME(Archive&, std::monostate const&) {}
} // namespace ser20

#endif // SER20_TYPES_STD_VARIANT_HPP_
