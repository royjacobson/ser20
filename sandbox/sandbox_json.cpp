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

#include <ser20/ser20.hpp>
#include <ser20/archives/binary.hpp>
#include <ser20/archives/json.hpp>

#include <ser20/types/string.hpp>
#include <ser20/types/utility.hpp>
#include <ser20/types/memory.hpp>
#include <ser20/types/complex.hpp>
#include <ser20/types/base_class.hpp>
#include <ser20/types/array.hpp>
#include <ser20/types/vector.hpp>
#include <ser20/types/map.hpp>

#include <sstream>
#include <fstream>
#include <cassert>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>

// ###################################
struct Test1
{
  int a;

  private:
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive & ar)
    {
      ar(SER20_NVP(a));
    }
};

// ###################################
class Test2
{
  public:
    Test2() {}
    Test2( int x ) : a( x ) {}
    int a;

  private:
    friend class ser20::access;

    template<class Archive>
      void save(Archive & ar) const
      {
        ar(a);
      }

    template<class Archive>
      void load(Archive & ar)
      {
        ar(a);
      }
};

// ###################################
struct Test3
{
  int a;
};

template<class Archive>
void serialize(Archive & ar, Test3 & t)
{
  ar(SER20_NVP(t.a));
}

namespace test4
{
  // ###################################
  struct Test4
  {
    int a;
  };

  template<class Archive>
  void save(Archive & ar, Test4 const & t)
  {
    ar(SER20_NVP(t.a));
  }

  template<class Archive>
  void load(Archive & ar, Test4 & t)
  {
    ar(SER20_NVP(t.a));
  }
}

class Private
{
  public:
    Private() : a('z') {}

  private:
    char a;

    friend class ser20::access;

    template<class Archive>
      void serialize(Archive & ar)
      {
        ar(a);
      }
};

struct Everything
{
  int x;
  int y;
  Test1 t1;
  Test2 t2;
  Test3 t3;
  test4::Test4 t4;
  std::string s;

  template<class Archive>
  void serialize(Archive & ar)
  {
    ar(SER20_NVP(x));
    ar(SER20_NVP(y));
    ar(SER20_NVP(t1));
    ar(SER20_NVP(t2));
    ar(SER20_NVP(t3));
    ar(SER20_NVP(t4));
    ar(SER20_NVP(s));
  }

  bool operator==(Everything const & o)
  {
    return
      x == o.x &&
      y == o.y &&
      t1.a == o.t1.a &&
      t2.a == o.t2.a &&
      t3.a == o.t3.a &&
      t4.a == o.t4.a &&
      s == o.s;
  }
};


struct SubFixture
{
  SubFixture() : a( 3 ),
    b( 9999 ),
    c( 100.1f ),
    d( 2000.9 ),
    s( "hello, world!" )
  {}

  int a;
  uint64_t b;
  float c;
  double d;
  std::string s;

    template<class Archive>
      void serialize(Archive & ar)
      {
        ar( SER20_NVP(a),
            b,
            c,
            SER20_NVP(d),
            SER20_NVP(s) );
      }
    void change()
    {
      a = 4;
      b = 4;
      c = 4;
      d = 4;
      s = "4";
    }
};

struct Fixture
{
  SubFixture f1, f2, f3;
  int array[4];

  Fixture()
  {
    array[0] = 1;
    array[1] = 2;
    array[2] = 3;
    array[3] = 4;
  }

  template<class Archive>
  void save(Archive & ar) const
  {
    ar( f1,
        SER20_NVP(f2),
        f3 );
    ar.saveBinaryValue( array, sizeof(int)*4, "cool array man" );
  }

  template<class Archive>
  void load(Archive & ar)
  {
    ar( f1,
        SER20_NVP(f2),
        f3 );
    ar.loadBinaryValue( array, sizeof(int)*4 );
  }

    void change()
    {
      f1.change();
      f2.change();
      f3.change();
    }
};

struct AAA
{
  AAA() : one( 1 ), two( 2 ), three( { {1, 2, 3}, { 4, 5, 6 }, {} } ) {}
  int one, two;

  std::vector<std::vector<int>> three;

  template<class Archive>
    void serialize(Archive & ar)
    {
      ar( SER20_NVP(one), SER20_NVP(two) );
      //ar( SER20_NVP(three) );
    }
};

class Stuff
{
  public:
    Stuff() {}

    void fillData()
    {
      std::vector<std::complex<float>> t1{ {0, -1.0f},
                          { 0, -2.9932f },
                          { 0, -3.5f } };
      std::vector<std::complex<float>> t2{ {1.0f, 0},
                     { 2.2f, 0 },
                     { 3.3f, 0 } };
      data["imaginary"] = t1;
      data["real"] = t2;
    }

  private:
    std::map<std::string, std::vector<std::complex<float>>> data;

    friend class ser20::access;

    template <class Archive>
    void serialize( Archive & ar )
    {
      ar( SER20_NVP(data) );
    }
};

struct OOJson
{
  OOJson() = default;
  OOJson( int aa, int bb, bool cc, double dd ) :
    a( aa ), b( bb ), c{ cc, dd }
  {
    d[0] = 0; d[1] = 1; d[2] = 2;
  }

  int a;
  int b;
  std::pair<bool, double> c;
  float d[3];

  template <class Archive>
  void serialize( Archive & ar )
  {
    ar( SER20_NVP(c) );
    ar( SER20_NVP(a) );
    ar( b );
    ar( SER20_NVP(d) );
  }
};

// ######################################################################
int main()
{
  std::cout << std::boolalpha << std::endl;

  {
    std::ofstream os("file.json");
    ser20::JSONOutputArchive oar( os );

    //auto f = std::make_shared<Fixture>();
    //auto f2 = f;
    //oar( f );
    //oar( f2 );
    Stuff s; s.fillData();
    oar( ser20::make_nvp("best data ever", s) );
  }

  {
    std::ifstream is("file.json");
    std::string str((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    std::cout << "---------------------" << std::endl << str << std::endl << "---------------------" << std::endl;
  }

  // playground
  {
    ser20::JSONOutputArchive archive( std::cout );
    bool arr[] = {true, false};
    std::vector<int> vec = {1, 2, 3, 4, 5};
    archive( SER20_NVP(vec),
        arr );
    auto f = std::make_shared<Fixture>();
    auto f2 = f;
    archive( f );
    archive( f2 );
  }

  // test out of order
  std::stringstream oos;
  {
    ser20::JSONOutputArchive ar(oos);
    ser20::JSONOutputArchive ar2(std::cout,
        ser20::JSONOutputArchive::Options(2, ser20::JSONOutputArchive::Options::IndentChar::space, 2) );

    ar( ser20::make_nvp( "1", 1 ),
        ser20::make_nvp( "2", 2 ),
        3,
        0, // unused
        ser20::make_nvp( "4", 4 ),
        ser20::make_nvp( "5", 5 ) );

    int x = 33;
    ar.saveBinaryValue( &x, sizeof(int), "bla" );

    ar2( ser20::make_nvp( "1", 1 ),
         ser20::make_nvp( "2", 2 ),
         3,
         0, // unused
         ser20::make_nvp( "4", 4 ),
         ser20::make_nvp( "5", 5 ) );
    ar2.saveBinaryValue( &x, sizeof(int), "bla" );

    OOJson oo( 1, 2, true, 4.2 );
    ar( SER20_NVP(oo) );
    ar2( SER20_NVP(oo) );

    // boost stuff
    ar & ser20::make_nvp("usingop&", oo ) & 6;
    ar << 5 << 4 << 3;

    ar2 & ser20::make_nvp("usingop&", oo ) & 6;
    ar2 << 5 << 4 << 3;

    long double ld = std::numeric_limits<long double>::max();
    long long ll = std::numeric_limits<long long>::max();
    unsigned long long ull = std::numeric_limits<unsigned long long>::max();

    ar( SER20_NVP(ld),
        SER20_NVP(ll),
        SER20_NVP(ull) );

    ar2( SER20_NVP(ld),
         SER20_NVP(ll),
         SER20_NVP(ull) );
  }

  {
    ser20::JSONInputArchive ar(oos);
    int i1, i2, i3, i4, i5, x;

    ar( i1 );
    ar( ser20::make_nvp( "2", i2 ), i3 );

    ar( ser20::make_nvp( "4", i4 ),
        i5 );

    ar.loadBinaryValue( &x, sizeof(int) );

    OOJson ii;
    ar( ser20::make_nvp("oo", ii) );
    ar( ser20::make_nvp( "2", i2 ) );

    std::cout << i1 << " " << i2 << " " << i3 << " " << i4 << " " << i5 << std::endl;
    std::cout << x << std::endl;
    std::cout << ii.a << " " << ii.b << " " << ii.c.first << " " << ii.c.second << " ";
    for( auto z : ii.d )
      std::cout << z << " ";
    std::cout << std::endl;

    OOJson oo;
    ar >> ser20::make_nvp("usingop&", oo );
    std::cout << oo.a << " " << oo.b << " " << oo.c.first << " " << oo.c.second << " ";
    for( auto z : oo.d )
      std::cout << z << " ";

    int aa, a, b, c;
    ar & aa & a & b & c;
    std::cout << aa << " " << a << " " << b << " " << c << std::endl;

    long double ld;
    long long ll;
    unsigned long long ull;

    ar( SER20_NVP(ld),
        SER20_NVP(ll),
        SER20_NVP(ull) );

    std::cout << (ld  == std::numeric_limits<long double>::max()) << std::endl;
    std::cout << (ll  == std::numeric_limits<long long>::max()) << std::endl;
    std::cout << (ull == std::numeric_limits<unsigned long long>::max()) << std::endl;
  }

  return 0;
}
