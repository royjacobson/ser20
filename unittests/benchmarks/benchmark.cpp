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

#include <iostream>
#include <random>
#include <sstream>

#include <ser20/archives/binary.hpp>
#include <ser20/types/map.hpp>
#include <ser20/types/string.hpp>
#include <ser20/types/vector.hpp>

#include "benchmark/benchmark.h"

struct ser20Binary {
  //! Saves data to a ser20 binary archive
  template <class T> static void save(std::ostringstream& os, T const& data) {
    ser20::BinaryOutputArchive oar(os);
    oar(data);
  }

  //! Loads data to a ser20 binary archive
  template <class T> static void load(std::istringstream& is, T& data) {
    ser20::BinaryInputArchive iar(is);
    iar(data);
  }
};

//! Times how long it takes to serialize (load and store) some data
template <class Data> void test(Data const& data, benchmark::State& state) {
  for (auto _ : state) {
    std::ostringstream os;
    Data deserialized_data;

    {
      ser20::BinaryOutputArchive oar(os);
      oar(data);
    }
    std::string x = os.str();

    std::istringstream is(x);
    {
      ser20::BinaryInputArchive iar(is);
      iar(deserialized_data);
    }
  }
}

template <class T>
typename std::enable_if<std::is_floating_point_v<T>, T>::type
random_value(std::mt19937& gen) {
  return std::uniform_real_distribution<T>(-10000.0, 10000.0)(gen);
}

template <class T>
typename std::enable_if<std::is_integral_v<T> && sizeof(T) != sizeof(char),
                        T>::type
random_value(std::mt19937& gen) {
  return std::uniform_int_distribution<T>(std::numeric_limits<T>::lowest(),
                                          std::numeric_limits<T>::max())(gen);
}

template <class T>
typename std::enable_if<std::is_integral_v<T> && sizeof(T) == sizeof(char),
                        T>::type
random_value(std::mt19937& gen) {
  return static_cast<T>(std::uniform_int_distribution<int64_t>(
      std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max())(gen));
}

template <class T>
typename std::enable_if<std::is_same_v<T, std::string>, std::string>::type
random_value(std::mt19937& gen) {
  std::string s(std::uniform_int_distribution<int>(3, 30)(gen), ' ');
  for (char& c : s)
    c = std::uniform_int_distribution<char>(' ', '~')(gen);
  return s;
}

template <class C>
std::basic_string<C> random_basic_string(std::mt19937& gen,
                                         size_t maxSize = 30) {
  std::basic_string<C> s(std::uniform_int_distribution<int>(3, maxSize)(gen),
                         ' ');
  for (C& c : s)
    c = static_cast<C>(std::uniform_int_distribution<int>('~', '~')(gen));
  return s;
  return s;
}

template <size_t N> std::string random_binary_string(std::mt19937& gen) {
  std::string s(N, ' ');
  for (auto& c : s)
    c = std::uniform_int_distribution<char>('0', '1')(gen);
  return s;
}

struct PoDStruct {
  int32_t a;
  int64_t b;
  float c;
  double d;

  template <class Archive> void serialize(Archive& ar) { ar(a, b, c, d); }
};

struct PoDChildSer20 : virtual PoDStruct {
  PoDChildSer20() : v(1024) {}

  std::vector<float> v;

  template <class Archive> void serialize(Archive& ar) {
    ar(ser20::virtual_base_class<PoDStruct>(this), v);
  }
};

static void BM_Vector_double(benchmark::State& state) {
  size_t size = state.range(0);
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<double> data(size);
  for (auto& d : data)
    d = random_value<double>(gen);

  test(data, state);
};

BENCHMARK(BM_Vector_double)->Args({1, 16, 1024, 1024 * 1024});

static void BM_Vector_uint8_t(benchmark::State& state) {
  size_t size = state.range(0);
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<uint8_t> data(size);
  for (auto& d : data)
    d = random_value<uint8_t>(gen);

  test(data, state);
}

BENCHMARK(BM_Vector_uint8_t)
    ->Args({1, 16, 1024, 1024 * 1024, 1024 * 1024 * 32});

static void BM_Vector_PoDStruct(benchmark::State& state) {
  size_t size = state.range(0);

  std::vector<PoDStruct> data(size);
  test(data, state);
};

BENCHMARK(BM_Vector_PoDStruct)
    ->Args({1, 64, 1024, 1024 * 1024, 1024 * 1024 * 2});

static void BM_Vector_PoDChild(benchmark::State& state) {
  size_t size = state.range(0);

  std::vector<PoDChildSer20> data(size);
  test(data, state);
};

BENCHMARK(BM_Vector_PoDChild)->Args({1024, 1024 * 32});

static void BM_String_size(benchmark::State& state) {
  size_t size = state.range(0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::string data = random_basic_string<char>(gen, size);
  test(data, state);
};

BENCHMARK(BM_String_size)->Args({200000, 2000000, 20000000});

static void BM_Vector_String(benchmark::State& state) {
  size_t size = state.range(0);

  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<std::string> data(size);
  for (size_t i = 0; i < data.size(); ++i)
    data[i] = random_basic_string<char>(gen);

  test(data, state);
};

BENCHMARK(BM_Vector_String)->Args({512, 1024, 1024 * 64, 1024 * 128});

static void BM_Map_PoDStruct(benchmark::State& state) {
  size_t size = state.range(0);

  std::map<std::string, PoDStruct> data;
  for (size_t i = 0; i < size; ++i) {
    data[std::to_string(i)] = PoDStruct();
  }
  test(data, state);
};

BENCHMARK(BM_Map_PoDStruct)->Args({1024, 1024 * 64, 1024 * 1024 * 2});

BENCHMARK_MAIN();
