/*
  Copyright (c) 2014, Randolph Voorhies, Shane Grant
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

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <base.hpp>
#include <derived.hpp>

#include <ser20/access.hpp>
#include <ser20/details/traits.hpp>
#include <ser20/details/helpers.hpp>
#include <ser20/types/base_class.hpp>
#include <ser20/ser20.hpp>

#include <ser20/types/array.hpp>
#include <ser20/types/bitset.hpp>
#include <ser20/types/chrono.hpp>
#include <ser20/types/common.hpp>
#include <ser20/types/complex.hpp>
#include <ser20/types/deque.hpp>
#include <ser20/types/forward_list.hpp>
#include <ser20/types/list.hpp>
#include <ser20/types/map.hpp>
#include <ser20/types/memory.hpp>

#include <ser20/details/util.hpp>

#include <ser20/details/polymorphic_impl.hpp>
#include <ser20/types/polymorphic.hpp>

#include <ser20/types/queue.hpp>
#include <ser20/types/set.hpp>
#include <ser20/types/stack.hpp>
#include <ser20/types/string.hpp>
#include <ser20/types/tuple.hpp>
#include <ser20/types/unordered_map.hpp>
#include <ser20/types/unordered_set.hpp>
#include <ser20/types/utility.hpp>
#include <ser20/types/vector.hpp>

#include <ser20/archives/binary.hpp>
#include <ser20/archives/portable_binary.hpp>
#include <ser20/archives/xml.hpp>
#include <ser20/archives/json.hpp>

#include <iostream>
#include <type_traits>
#include <functional>

//SER20_FORCE_LINK_SHARED_LIBRARY(Sandbox)

struct Archive {};
SER20_SETUP_ARCHIVE_TRAITS(Archive, Archive)

struct Test
{
  template <class Archive>
  void serialzize( Archive & )
  {
    std::cout << "hey there" << std::endl;
  }

  template <class Archive>
  void save( Archive & ) const
  {
    std::cout << "saved by the bell" << std::endl;
  }

  template <class Archive>
  void load( Archive & )
  {
    std::cout << "locked and loaded" << std::endl;
  }

  template <class Archive>
  static void load_and_construct( Archive &, ser20::construct<Test> & )
  {
  }

  template <class Archive>
  int save_minimal() const
  {
    return 0;
  }

  template <class Archive>
  int save_minimal(const std::uint32_t) const
  {
    return 1;
  }

  template <class Archive>
  void load_minimal( int & )
  { }
};

template <class Archive>
void serialize( Archive &, Test & )
{ }

template <class Archive>
void load( Archive &, Test & )
{ }

template <class Archive>
void save( Archive &, Test const & )
{ }

template <class Archive>
int save_minimal( Test const & )
{ return 0; }

template <class Archive>
int save_minimal( Test const &, const std::uint32_t )
{ return 0; }

namespace ser20
{
  template <>
  struct LoadAndConstruct<Test>
  {
    template <class Archive>
    static void load_and_construct( Archive &, ser20::construct<Test> & construct )
    {
      construct();
    }
  };
}

struct A
{
  virtual void foo() = 0;
  virtual ~A() {}
};

struct B : A
{
  virtual ~B() {}
  void foo() {}

  template <class Archive>
  void serialize( Archive & )
  {
    std::cout << "i'm in your b" << std::endl;
  }
};

struct C
{
  char a;
};

SER20_REGISTER_TYPE(B)
SER20_REGISTER_POLYMORPHIC_RELATION(A, B)

class MemberMinimal
{
  public:
    MemberMinimal() = default;
    template <class Archive>
    int save_minimal( Archive const & ) const
    {
      return x;
    }

    template <class Archive>
    void load_minimal( Archive const &, int const & str )
    {
      x = str;
    }

  public:
    int x;
};

int main()
{
  typedef Test T;
  std::cout << std::boolalpha;

  // Test Load and Construct internal/external
  std::cout << "\tload_and_construct" << std::endl;
  std::cout << ser20::traits::has_member_load_and_construct_v<T, Archive> << std::endl;
  std::cout << ser20::traits::has_non_member_load_and_construct_v<T, Archive> << std::endl;

  // serialize
  std::cout << "\tserialize" << std::endl;
  std::cout << ser20::traits::has_member_serialize_v<T, Archive> << std::endl;
  std::cout << ser20::traits::has_non_member_serialize_v<T, Archive> << std::endl;

  // load
  std::cout << "\tload" << std::endl;
  std::cout << ser20::traits::has_member_load_v<T, Archive> << std::endl;
  std::cout << ser20::traits::has_non_member_load_v<T, Archive> << std::endl;

  // load minimal
  std::cout << "\tload minimal" << std::endl;
  std::cout << ser20::traits::has_member_load_v<T, Archive> << std::endl;

  // save
  std::cout << "\tsave" << std::endl;
  std::cout << ser20::traits::has_member_save_v<T, Archive> << std::endl;
  std::cout << ser20::traits::has_non_member_save_v<T, Archive> << std::endl;

  // save_minimal
  std::cout << "\tsave_minimal" << std::endl;
  std::cout << ser20::traits::has_member_save_minimal_v<T, Archive> << std::endl;
  std::cout << ser20::traits::has_non_member_save_minimal_v<T, Archive> << std::endl;

  // save_minimal_versioned
  std::cout << "\tsave_minimal versioned" << std::endl;
  std::cout << ser20::traits::has_member_versioned_save_minimal_v<T, Archive> << std::endl;
  std::cout << ser20::traits::has_non_member_versioned_save_minimal_v<T, Archive> << std::endl;

  // splittable
  std::cout << "\t splittable" << std::endl;
  std::cout << ser20::traits::has_member_split_v<T, Archive, Archive> << std::endl;
  std::cout << ser20::traits::has_non_member_split_v<T, Archive, Archive> << std::endl;

  // serialiable
  std::cout << "\toutput serializable" << std::endl;
  std::cout << ser20::traits::is_output_serializable_v<T, Archive> << std::endl;

#if !defined(__INTEL_COMPILER)
  //! TODO: This causes icc to crash
  std::cout << ser20::traits::is_input_serializable_v<T, Archive> << std::endl;
#endif

  // specialized
  std::cout << "\tspecialized" << std::endl;
  std::cout << ser20::traits::detail::is_specialized_member_serialize_v<T, Archive> << std::endl;
  std::cout << ser20::traits::detail::is_specialized_member_load_save_v<T, Archive> << std::endl;
  std::cout << ser20::traits::detail::is_specialized_non_member_serialize_v<T, Archive> << std::endl;
  std::cout << ser20::traits::detail::is_specialized_non_member_load_save_v<T, Archive> << std::endl;
  std::cout << ser20::traits::detail::count_specializations<T, Archive> << std::endl;
  std::cout << ser20::traits::is_specialized_v<T, Archive> << std::endl;

  // array size
  std::cout << typeid(A).name() << std::endl;
  std::cout << ser20::traits::has_load_and_construct_v<int, bool> << std::endl;

  // extra testing
  std::cout << "\textra" << std::endl;
  std::cout << ser20::traits::has_member_save_minimal_v<MemberMinimal, Archive> << std::endl;
  std::cout << ser20::traits::has_member_load_minimal_v<MemberMinimal, Archive> << std::endl;

  // DLL testing
  std::cout << "------DLL TESTING------" << std::endl;
  std::stringstream dllSS1;
  std::stringstream dllSS2;
  {
    ser20::XMLOutputArchive out(dllSS1);
    VersionTest x{1};
    std::shared_ptr<Base> p = std::make_shared<Derived>();
    out(x);
    out( p );

    std::shared_ptr<A> ay = std::make_shared<B>();
    out(ay);
  }

  std::cout << dllSS1.str() << std::endl;

  {
    VersionTest x;
    std::shared_ptr<Base> p;
    std::shared_ptr<A> ay;
    {
      ser20::XMLInputArchive in(dllSS1);
      in(x);
      in( p );
      in( ay );
    }
    {
      ser20::XMLOutputArchive out(dllSS2);
      out( x );
      out( p );
      out( ay );
    }
  }

  std::cout << dllSS2.str() << std::endl;

  return 0;
}
