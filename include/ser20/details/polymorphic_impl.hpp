/*! \file polymorphic_impl.hpp
    \brief Internal polymorphism support
    \ingroup Internal */
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

/* This code is heavily inspired by the boost serialization implementation by
   the following authors

   (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
   Use, modification and distribution is subject to the Boost Software
   License, Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt)

    See http://www.boost.org for updates, documentation, and revision history.

   (C) Copyright 2006 David Abrahams - http://www.boost.org.

   See /boost/serialization/export.hpp,
   /boost/archive/detail/register_archive.hpp, and
   /boost/serialization/void_cast.hpp for their implementation. Additional
   details found in other files split across serialization and archive.
*/
#ifndef SER20_DETAILS_POLYMORPHIC_IMPL_HPP_
#define SER20_DETAILS_POLYMORPHIC_IMPL_HPP_

#include "ser20/details/polymorphic_impl_fwd.hpp"
#include "ser20/details/static_object.hpp"
#include "ser20/details/util.hpp"
#include "ser20/types/memory.hpp"
#include "ser20/types/string.hpp"
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <stack>
#include <typeindex>

//! Helper macro to omit unused warning
#if defined(__GNUC__)
// GCC / clang don't want the function
#define SER20_BIND_TO_ARCHIVES_UNUSED_FUNCTION
#else
#define SER20_BIND_TO_ARCHIVES_UNUSED_FUNCTION                                \
  static void unused() { (void)b; }
#endif

//! Binds a polymorhic type to all registered archives
/*! This binds a polymorphic type to all compatible registered archives that
    have been registered with SER20_REGISTER_ARCHIVE.  This must be called
    after all archives are registered (usually after the archives themselves
    have been included). */
#define SER20_BIND_TO_ARCHIVES(...)                                           \
  namespace ser20 {                                                           \
  namespace detail {                                                           \
  template <> struct init_binding<__VA_ARGS__> {                               \
    static inline bind_to_archives<__VA_ARGS__> const& b =                     \
        ::ser20::detail::StaticObject<                                        \
            bind_to_archives<__VA_ARGS__>>::getInstance()                      \
            .bind();                                                           \
    SER20_BIND_TO_ARCHIVES_UNUSED_FUNCTION                                    \
  };                                                                           \
  }                                                                            \
  } /* end namespaces */

namespace ser20 {
/* Polymorphic casting support */
namespace detail {
//! Base type for polymorphic void casting
/*! Contains functions for casting between registered base and derived types.

    This is necessary so that ser20 can properly cast between polymorphic types
    even though void pointers are used, which normally have no type information.
    Runtime type information is used instead to index a compile-time made
   mapping that can perform the proper cast. In the case of multiple levels of
   inheritance, ser20 will attempt to find the shortest path by using
   registered relationships to perform the cast.

    This class will be allocated as a StaticObject and only referenced by
   pointer, allowing a templated derived version of it to define strongly typed
   functions that cast between registered base and derived types. */
struct PolymorphicCaster {
  PolymorphicCaster();
  PolymorphicCaster(const PolymorphicCaster&);
  PolymorphicCaster& operator=(const PolymorphicCaster&);
  PolymorphicCaster(PolymorphicCaster&&) noexcept;
  PolymorphicCaster& operator=(PolymorphicCaster&&) noexcept;
  virtual ~PolymorphicCaster() noexcept;

  //! Downcasts to the proper derived type
  virtual void const* downcast(void const* const ptr) const = 0;
  //! Upcast to proper base type
  virtual void* upcast(void* const ptr) const = 0;
  //! Upcast to proper base type, shared_ptr version
  virtual std::shared_ptr<void>
  upcast(std::shared_ptr<void> const& ptr) const = 0;
};

//! Holds registered mappings between base and derived types for casting
/*! This will be allocated as a StaticObject and holds a map containing
    all registered mappings between base and derived types. */
struct PolymorphicCasters {
  //! Maps from a derived type index to a set of chainable casters
  using DerivedCasterMap =
      std::unordered_map<std::type_index,
                         std::vector<PolymorphicCaster const*>>;
  //! Maps from base type index to a map from derived type index to caster
  std::unordered_map<std::type_index, DerivedCasterMap> map;

  std::multimap<std::type_index, std::type_index> reverseMap;

//! Error message used for unregistered polymorphic casts
#define UNREGISTERED_POLYMORPHIC_CAST_EXCEPTION(LoadSave)                      \
  throw ser20::Exception(                                                     \
      "Trying to " #LoadSave " a registered polymorphic type with an "         \
      "unregistered polymorphic cast.\n"                                       \
      "Could not find a path to a base class (" +                              \
      util::demangle(baseInfo.name()) +                                        \
      ") for type: " + ::ser20::util::demangledName<Derived>() +              \
      "\n"                                                                     \
      "Make sure you either serialize the base class at some point via "       \
      "ser20::base_class or ser20::virtual_base_class.\n"                    \
      "Alternatively, manually register the association with "                 \
      "SER20_REGISTER_POLYMORPHIC_RELATION.");

  //! Checks if the mapping object that can perform the upcast or downcast
  //! exists, and returns it if so
  /*! Uses the type index from the base and derived class to find the matching
      registered caster. If no matching caster exists, the bool in the pair will
      be false and the vector reference should not be used. */
  static std::pair<bool, std::vector<PolymorphicCaster const*> const&>
  lookup_if_exists(std::type_index const& baseIndex,
                   std::type_index const& derivedIndex);

  //! Gets the mapping object that can perform the upcast or downcast
  /*! Uses the type index from the base and derived class to find the matching
      registered caster. If no matching caster exists, returns null_ptr.

      The returned PolymorphicCaster is capable of upcasting or downcasting
      between the two types. */
  static const std::vector<PolymorphicCaster const*>*
  lookup(const std::type_index& baseIndex, const std::type_index& derivedIndex);

  //! Performs a downcast to the derived type using a registered mapping
  template <class Derived>
  inline static const Derived* downcast(const void* dptr,
                                        std::type_info const& baseInfo) {
    const auto* mapping = lookup(baseInfo, typeid(Derived));

    if (!mapping)
      UNREGISTERED_POLYMORPHIC_CAST_EXCEPTION(save);

    for (auto const* dmap : *mapping)
      dptr = dmap->downcast(dptr);

    return static_cast<Derived const*>(dptr);
  }

  //! Performs an upcast to the registered base type using the given a derived
  //! type
  /*! The return is untyped because the final casting to the base type must
     happen in the polymorphic serialization function, where the type is known
     at compile time */
  template <class Derived>
  inline static void* upcast(Derived* const dptr,
                             std::type_info const& baseInfo) {
    const auto* mapping = lookup(baseInfo, typeid(Derived));

    if (!mapping)
      UNREGISTERED_POLYMORPHIC_CAST_EXCEPTION(load);

    void* uptr = dptr;
    for (auto mIter = mapping->rbegin(), mEnd = mapping->rend(); mIter != mEnd;
         ++mIter)
      uptr = (*mIter)->upcast(uptr);

    return uptr;
  }

  //! Upcasts for shared pointers
  template <class Derived>
  inline static std::shared_ptr<void>
  upcast(std::shared_ptr<Derived> const& dptr, std::type_info const& baseInfo) {
    const auto* mapping = lookup(baseInfo, typeid(Derived));

    if (!mapping)
      UNREGISTERED_POLYMORPHIC_CAST_EXCEPTION(load);

    std::shared_ptr<void> uptr = dptr;
    for (auto mIter = mapping->rbegin(), mEnd = mapping->rend(); mIter != mEnd;
         ++mIter)
      uptr = (*mIter)->upcast(uptr);

    return uptr;
  }

#undef UNREGISTERED_POLYMORPHIC_CAST_EXCEPTION
};

void register_virtual_caster(const std::type_index baseKey,
                             const std::type_index derivedKey,
                             PolymorphicCaster* caster);

//! Strongly typed derivation of PolymorphicCaster
template <class Base, class Derived>
struct PolymorphicVirtualCaster : PolymorphicCaster {
  //! Inserts an entry in the polymorphic casting map for this pairing
  /*! Creates an explicit mapping between Base and Derived in both upwards and
      downwards directions, allowing void pointers to either to be properly cast
      assuming dynamic type information is available */
  PolymorphicVirtualCaster() {
    const auto baseKey = std::type_index(typeid(Base));
    const auto derivedKey = std::type_index(typeid(Derived));
    register_virtual_caster(baseKey, derivedKey, this);
  }

  //! Performs the proper downcast with the templated types
  void const* downcast(void const* const ptr) const override {
    return dynamic_cast<Derived const*>(static_cast<Base const*>(ptr));
  }

  //! Performs the proper upcast with the templated types
  void* upcast(void* const ptr) const override {
    return dynamic_cast<Base*>(static_cast<Derived*>(ptr));
  }

  //! Performs the proper upcast with the templated types (shared_ptr version)
  std::shared_ptr<void>
  upcast(std::shared_ptr<void> const& ptr) const override {
    return std::dynamic_pointer_cast<Base>(
        std::static_pointer_cast<Derived>(ptr));
  }
};

//! Registers a polymorphic casting relation between a Base and Derived type
/*! Registering a relation allows ser20 to properly cast between the two types
    given runtime type information and void pointers.

    Registration happens automatically via ser20::base_class and
    ser20::virtual_base_class instantiations. For cases where neither is
   called, see the SER20_REGISTER_POLYMORPHIC_RELATION macro */
template <class Base, class Derived> struct RegisterPolymorphicCaster {
  //! Performs registration (binding) between Base and Derived
  /*! If the type is not polymorphic, nothing will happen */
  inline static PolymorphicCaster const* bind() {
    return std::is_polymorphic_v<Base>
               ? &StaticObject<
                     PolymorphicVirtualCaster<Base, Derived>>::getInstance()
               : nullptr;
  }
};
} // namespace detail

/* General polymorphism support */
namespace detail {
//! Binds a compile time type with a user defined string
template <class T> struct binding_name {};

//! A structure holding a map from type_indices to output serializer functions
/*! A static object of this map should be created for each registered archive
    type, containing entries for every registered type that describe how to
    properly cast the type to its real type in polymorphic scenarios for
    shared_ptr, weak_ptr, and unique_ptr. */
template <class Archive> struct OutputBindingMap {
  //! A serializer function
  /*! Serializer functions return nothing and take an archive as
      their first parameter (will be cast properly inside the function,
      a pointer to actual data (contents of smart_ptr's get() function)
      as their second parameter, and the type info of the owning smart_ptr
      as their final parameter */
  typedef std::function<void(void*, void const*, std::type_info const&)>
      Serializer;

  //! Struct containing the serializer functions for all pointer types
  struct Serializers {
    Serializer shared_ptr, //!< Serializer function for shared/weak pointers
        unique_ptr;        //!< Serializer function for unique pointers
  };

  //! A map of serializers for pointers of all registered types
  std::map<std::type_index, Serializers> map;
};

//! An empty noop deleter
template <class T> struct EmptyDeleter {
  void operator()(T*) const {}
};

//! A structure holding a map from type name strings to input serializer
//! functions
/*! A static object of this map should be created for each registered archive
    type, containing entries for every registered type that describe how to
    properly cast the type to its real type in polymorphic scenarios for
    shared_ptr, weak_ptr, and unique_ptr. */
template <class Archive> struct InputBindingMap {
  //! Shared ptr serializer function
  /*! Serializer functions return nothing and take an archive as
      their first parameter (will be cast properly inside the function,
      a shared_ptr (or unique_ptr for the unique case) of any base
      type, and the type id of said base type as the third parameter.
      Internally it will properly be loaded and cast to the correct type. */
  typedef std::function<void(void*, std::shared_ptr<void>&,
                             std::type_info const&)>
      SharedSerializer;
  //! Unique ptr serializer function
  typedef std::function<void(void*, std::unique_ptr<void, EmptyDeleter<void>>&,
                             std::type_info const&)>
      UniqueSerializer;

  //! Struct containing the serializer functions for all pointer types
  struct Serializers {
    SharedSerializer
        shared_ptr; //!< Serializer function for shared/weak pointers
    UniqueSerializer unique_ptr; //!< Serializer function for unique pointers
  };

  //! A map of serializers for pointers of all registered types
  std::map<std::string, Serializers> map;
};

// forward decls for archives from ser20.hpp
class InputArchiveBase;
class OutputArchiveBase;

//! Creates a binding (map entry) between an input archive type and a
//! polymorphic type
/*! Bindings are made when types are registered, assuming that at least one
    archive has already been registered.  When this struct is created,
    it will insert (at run time) an entry into a map that properly handles
    casting for serializing polymorphic objects */
template <class Archive, class T> struct InputBindingCreator {
  //! Initialize the binding
  InputBindingCreator() {
    auto& map = StaticObject<InputBindingMap<Archive>>::getInstance().map;
    auto lock = StaticObject<InputBindingMap<Archive>>::lock();
    auto key = std::string(binding_name<T>::name());
    auto lb = map.lower_bound(key);

    if (lb != map.end() && lb->first == key)
      return;

    typename InputBindingMap<Archive>::Serializers serializers;

    serializers.shared_ptr = [](void* arptr, std::shared_ptr<void>& dptr,
                                std::type_info const& baseInfo) {
      Archive& ar = *static_cast<Archive*>(arptr);
      std::shared_ptr<T> ptr;

      ar(SER20_NVP_("ptr_wrapper",
                     ::ser20::memory_detail::make_ptr_wrapper(ptr)));

      dptr = PolymorphicCasters::template upcast<T>(ptr, baseInfo);
    };

    serializers.unique_ptr = [](void* arptr,
                                std::unique_ptr<void, EmptyDeleter<void>>& dptr,
                                std::type_info const& baseInfo) {
      Archive& ar = *static_cast<Archive*>(arptr);
      std::unique_ptr<T> ptr;

      ar(SER20_NVP_("ptr_wrapper",
                     ::ser20::memory_detail::make_ptr_wrapper(ptr)));

      dptr.reset(
          PolymorphicCasters::template upcast<T>(ptr.release(), baseInfo));
    };

    map.insert(lb, {std::move(key), std::move(serializers)});
  }
};

//! Creates a binding (map entry) between an output archive type and a
//! polymorphic type
/*! Bindings are made when types are registered, assuming that at least one
    archive has already been registered.  When this struct is created,
    it will insert (at run time) an entry into a map that properly handles
    casting for serializing polymorphic objects */
template <class Archive, class T> struct OutputBindingCreator {
  //! Writes appropriate metadata to the archive for this polymorphic type
  static inline void writeMetadata(Archive& ar) {
    // Register the polymorphic type name with the archive, and get the id
    char const* name = binding_name<T>::name();
    std::uint32_t id = ar.registerPolymorphicType(name);

    // Serialize the id
    ar(SER20_NVP_("polymorphic_id", id));

    // If the msb of the id is 1, then the type name is new, and we should
    // serialize it
    if (id & msb_32bit) {
      std::string namestring(name);
      ar(SER20_NVP_("polymorphic_name", namestring));
    }
  }

  //! Holds a properly typed shared_ptr to the polymorphic type
  class PolymorphicSharedPointerWrapper {
  public:
    /*! Wrap a raw polymorphic pointer in a shared_ptr to its true type

        The wrapped pointer will not be responsible for ownership of the held
        pointer so it will not attempt to destroy it; instead the refcount of
        the wrapped pointer will be tied to a fake 'ownership pointer' that will
        do nothing when it ultimately goes out of scope.

        The main reason for doing this, other than not to destroy the true
        object with our wrapper pointer, is to avoid meddling with the internal
        reference count in a polymorphic type that inherits from
        std::enable_shared_from_this.

        @param dptr A void pointer to the contents of the shared_ptr to
                    serialize */
    PolymorphicSharedPointerWrapper(T const* dptr)
        : refCount(), wrappedPtr(refCount, dptr) {}

    //! Get the wrapped shared_ptr */
    inline std::shared_ptr<T const> const& operator()() const {
      return wrappedPtr;
    }

  private:
    std::shared_ptr<void> refCount;      //!< The ownership pointer
    std::shared_ptr<T const> wrappedPtr; //!< The wrapped pointer
  };

  //! Does the actual work of saving a polymorphic shared_ptr
  /*! This function will properly create a shared_ptr from the void * that is
      passed in before passing it to the archive for serialization.

      In addition, this will also preserve the state of any internal
      enable_shared_from_this mechanisms

      @param ar The archive to serialize to
      @param dptr Pointer to the actual data held by the shared_ptr */
  static inline void
  savePolymorphicSharedPtr(Archive& ar, T const* dptr,
                           std::true_type /* has_shared_from_this */) {
    ::ser20::memory_detail::EnableSharedStateHelper<T> state(
        const_cast<T*>(dptr));
    PolymorphicSharedPointerWrapper psptr(dptr);
    ar(SER20_NVP_("ptr_wrapper", memory_detail::make_ptr_wrapper(psptr())));
  }

  //! Does the actual work of saving a polymorphic shared_ptr
  /*! This function will properly create a shared_ptr from the void * that is
      passed in before passing it to the archive for serialization.

      This version is for types that do not inherit from
      std::enable_shared_from_this.

      @param ar The archive to serialize to
      @param dptr Pointer to the actual data held by the shared_ptr */
  static inline void
  savePolymorphicSharedPtr(Archive& ar, T const* dptr,
                           std::false_type /* has_shared_from_this */) {
    PolymorphicSharedPointerWrapper psptr(dptr);
    ar(SER20_NVP_("ptr_wrapper", memory_detail::make_ptr_wrapper(psptr())));
  }

  //! Initialize the binding
  OutputBindingCreator() {
    auto& map = StaticObject<OutputBindingMap<Archive>>::getInstance().map;
    auto key = std::type_index(typeid(T));
    auto lb = map.lower_bound(key);

    if (lb != map.end() && lb->first == key)
      return;

    typename OutputBindingMap<Archive>::Serializers serializers;

    serializers.shared_ptr = [&](void* arptr, void const* dptr,
                                 std::type_info const& baseInfo) {
      Archive& ar = *static_cast<Archive*>(arptr);
      writeMetadata(ar);

      auto ptr = PolymorphicCasters::template downcast<T>(dptr, baseInfo);

      savePolymorphicSharedPtr(
          ar, ptr,
          std::integral_constant<bool,
                                 ::ser20::traits::has_shared_from_this<T>>());
    };

    serializers.unique_ptr = [&](void* arptr, void const* dptr,
                                 std::type_info const& baseInfo) {
      Archive& ar = *static_cast<Archive*>(arptr);
      writeMetadata(ar);

      std::unique_ptr<T const, EmptyDeleter<T const>> const ptr(
          PolymorphicCasters::template downcast<T>(dptr, baseInfo));

      ar(SER20_NVP_("ptr_wrapper", memory_detail::make_ptr_wrapper(ptr)));
    };

    map.insert({std::move(key), std::move(serializers)});
  }
};

//! Used to help out argument dependent lookup for finding potential overloads
//! of instantiate_polymorphic_binding
struct adl_tag {};

//! Causes the static object bindings between an archive type and a serializable
//! type T
template <class Archive, class T> struct create_bindings {
  static const InputBindingCreator<Archive, T>& load(std::true_type) {
    return ser20::detail::StaticObject<
        InputBindingCreator<Archive, T>>::getInstance();
  }

  static const OutputBindingCreator<Archive, T>& save(std::true_type) {
    return ser20::detail::StaticObject<
        OutputBindingCreator<Archive, T>>::getInstance();
  }

  inline static void load(std::false_type) {}
  inline static void save(std::false_type) {}
};

//! When specialized, causes the compiler to instantiate its parameter
template <void (*)()> struct instantiate_function {};

/*! This struct is used as the return type of instantiate_polymorphic_binding
    for specific Archive types.  When the compiler looks for overloads of
    instantiate_polymorphic_binding, it will be forced to instantiate this
    struct during overload resolution, even though it will not be part of a
    valid overload */
template <class Archive, class T> struct polymorphic_serialization_support {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  //! Creates the appropriate bindings depending on whether the archive supports
  //! saving or loading
  virtual SER20_DLL_EXPORT void instantiate() SER20_USED;
#else  // NOT _MSC_VER
  //! Creates the appropriate bindings depending on whether the archive supports
  //! saving or loading
  inline static SER20_DLL_EXPORT void instantiate() SER20_USED;
  //! This typedef causes the compiler to instantiate this static function
  typedef instantiate_function<instantiate> unused;
#endif // _MSC_VER
};

// instantiate implementation
template <class Archive, class T>
SER20_DLL_EXPORT void
polymorphic_serialization_support<Archive, T>::instantiate() {
  create_bindings<Archive, T>::save(
      std::integral_constant < bool,
      std::is_base_of_v<OutputArchiveBase, Archive>&&
              traits::is_output_serializable_v < T,
      Archive >> {});

  create_bindings<Archive, T>::load(
      std::integral_constant < bool,
      std::is_base_of_v<InputArchiveBase, Archive>&&
              traits::is_input_serializable_v < T,
      Archive >> {});
}

//! Begins the binding process of a type to all registered archives
/*! Archives need to be registered prior to this struct being instantiated via
    the SER20_REGISTER_ARCHIVE macro.  Overload resolution will then force
    several static objects to be made that allow us to bind together all
    registered archive types with the parameter type T. */
template <class T> struct bind_to_archives {
  //! Binds the type T to all registered archives
  /*! If T is abstract, we will not serialize it and thus
      do not need to make a binding */
  bind_to_archives const& bind() const {
    static_assert(std::is_polymorphic_v<T>,
                  "Attempting to register non polymorphic type");
    if constexpr (!std::is_abstract_v<T>) {
      instantiate_polymorphic_binding(static_cast<T*>(nullptr), 0, adl_tag{});
    }
    return *this;
  }
};

//! Used to hide the static object used to bind T to registered archives
template <class T> struct init_binding;

//! Base case overload for instantiation
/*! This will end up always being the best overload due to the second
    parameter always being passed as an int.  All other overloads will
    accept pointers to archive types and have lower precedence than int.

    Since the compiler needs to check all possible overloads, the
    other overloads created via SER20_REGISTER_ARCHIVE, which will have
    lower precedence due to requring a conversion from int to (Archive*),
    will cause their return types to be instantiated through the static object
    mechanisms even though they are never called.

    See the documentation for the other functions to try and understand this */
template <class T> void instantiate_polymorphic_binding(T*, int, adl_tag) {}
} // namespace detail
} // namespace ser20

#endif // SER20_DETAILS_POLYMORPHIC_IMPL_HPP_
