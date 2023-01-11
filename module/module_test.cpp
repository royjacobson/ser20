// To run:
// clang++ --std=c++20 --precompile ../module/ser20.cppm -I../include -fprebuilt-module-path=. -o ser20.pcm
// clang++ --std=c++20 -c ser20.pcm -o ser20.o
// clang++ --std=c++20 ../module/module_test.cpp ser20.o -fprebuilt-module-path=. -o module_test

#include <sstream>
#include <iostream>
import ser20;

#define TYPE_WITH_SERIALIZATION(name)                                          \
  struct name {                                                                \
    int a;                                                                     \
    float b;                                                                   \
    double c;                                                                  \
    int x;                                                                     \
    template <class Archive> void serialize(Archive& ar) {                     \
      ar& a;                                                                   \
      ar& b;                                                                   \
      ar& c;                                                                   \
      ar& x;                                                                   \
    }                                                                          \
  };                                                                           \
  void instantiate_##name(ser20::JSONOutputArchive& oa,                        \
                          ser20::JSONInputArchive& ia) {                       \
    name x;                                                                    \
    ia(x);                                                                     \
    oa(x);                                                                     \
  }

TYPE_WITH_SERIALIZATION(struct1);
TYPE_WITH_SERIALIZATION(struct2);

int main() {
  struct1 a{1, 1.3f, 1.4, 5};
  struct2 b{2, 2.3f, 2.4, 6};

  std::ostringstream os;
  ser20::BinaryOutputArchive oar(os);

  oar(a, b);

  std::string x = os.str();
  std::istringstream is(x);

  ser20::BinaryInputArchive iar(is);
  iar(a, b);

  std::cout << "a:  " << a.a << ", " << a.b << ", " << a.c << ", " << a.x << std::endl;
  std::cout << "b:  " << b.a << ", " << b.b << ", " << b.c << ", " << b.x << std::endl;
}
