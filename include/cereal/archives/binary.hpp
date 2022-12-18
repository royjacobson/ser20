/*! \file binary.hpp
    \brief Binary input and output archives */
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
#ifndef CEREAL_ARCHIVES_BINARY_HPP_
#define CEREAL_ARCHIVES_BINARY_HPP_

#include "cereal/cereal.hpp"
#include <cstring>
#include <sstream>
#include <cassert>

namespace cereal {
// ######################################################################
//! An output archive designed to save data in a compact binary representation
/*! This archive outputs data to a stream in an extremely compact binary
    representation with as little extra metadata as possible.

    This archive does nothing to ensure that the endianness of the saved
    and loaded data is the same.  If you need to have portability over
    architectures with different endianness, use PortableBinaryOutputArchive.

    When using a binary archive and a file stream, you must use the
    std::ios::binary format flag to avoid having your data altered
    inadvertently.

    \ingroup Archives */
class BinaryOutputArchive
    : public OutputArchive<BinaryOutputArchive, AllowEmptyClassElision> {
public:
  //! Construct, outputting to the provided stream
  /*! @param stream The stream to output to.  Can be a stringstream, a file
     stream, or even cout! */
  BinaryOutputArchive(std::ostream& stream, size_t bufferCapacity = 0x1000)
      : OutputArchive<BinaryOutputArchive, AllowEmptyClassElision>(this),
        itsStream(stream), bufferCapacity(bufferCapacity), bufferSize(0),
        itsBuffer(bufferCapacity) {}

  ~BinaryOutputArchive() {
    if (bufferSize) {
      flush();
    }
  }

  //! Writes size bytes of data to the output stream
  void saveBinary(const void* data, std::streamsize size) {
    auto as_char = reinterpret_cast<const char*>(data);
    if (4 * size > bufferCapacity) {
      flush();
      write(as_char, size);
    } else {
      if (bufferSize + size > bufferCapacity) {
        flush();
      }
      std::memcpy(itsBuffer.data() + bufferSize, as_char, size);
      bufferSize += size;
    }
  }

private:
  void flush() {
    write(itsBuffer.data(), bufferSize);
    bufferSize = 0;
  }

  void write(const char* data, std::streamsize size) {
    auto const writtenSize = itsStream.rdbuf()->sputn(data, size);
    if (writtenSize != size)
      throw Exception("Failed to write " + std::to_string(size) +
                      " bytes to output stream! Wrote " +
                      std::to_string(writtenSize));
  }

  std::ostream& itsStream;

  size_t bufferCapacity;
  size_t bufferSize;
  std::vector<char> itsBuffer;
};

// ######################################################################
//! An input archive designed to load data saved using BinaryOutputArchive
/*  This archive does nothing to ensure that the endianness of the saved
    and loaded data is the same.  If you need to have portability over
    architectures with different endianness, use PortableBinaryOutputArchive.

    When using a binary archive and a file stream, you must use the
    std::ios::binary format flag to avoid having your data altered
    inadvertently.

    \ingroup Archives */
class BinaryInputArchive
    : public InputArchive<BinaryInputArchive, AllowEmptyClassElision> {
public:
  //! Construct, loading from the provided stream
  BinaryInputArchive(std::istream& stream, size_t bufferCapacity = 0x1000)
      : InputArchive<BinaryInputArchive, AllowEmptyClassElision>(this),
        itsStream(stream), bufferCapacity(bufferCapacity), bufferStart(0),
        bufferEnd(0), itsBuffer(bufferCapacity) {}

  ~BinaryInputArchive() noexcept = default;

  //! Reads size bytes of data from the input stream
  void loadBinary(void* const data, std::streamsize size) {
    auto* as_chars = reinterpret_cast<char*>(data);

    auto bytesRead = flush(as_chars, size);
    if (bytesRead == size) {
      return;
    }

    loadFromStream(as_chars + bytesRead, size - bytesRead);
  }

private:
  std::streamsize flush(char* data, std::streamsize size) {
    auto size_to_read =
        std::min<std::streamsize>(bufferEnd - bufferStart, size);
    std::memcpy(data, itsBuffer.data() + bufferStart, size_to_read);
    bufferStart += size_to_read;
    return size_to_read;
  }

  // Can only be called when the buffer is empty!
  void loadFromStream(char* data, std::streamsize size) {
    assert(bufferEnd == bufferStart);

    if (4 * size > bufferCapacity) {
      load(data, size, size);
    } else {
      bufferStart = 0;
      bufferEnd = load(itsBuffer.data(), bufferCapacity, size);
      auto bytesRead = flush(data, size);
      assert(bytesRead == size);
    }
  }

  std::streamsize load(char* data, std::streamsize size,
                       std::streamsize min_required) {
    auto const readSize = itsStream.rdbuf()->sgetn(data, size);

    if (readSize < min_required)
      throw Exception("Failed to read " + std::to_string(min_required) +
                      " bytes from input stream! Read " +
                      std::to_string(readSize));
    return readSize;
  }

  std::istream& itsStream;

  size_t bufferCapacity;
  size_t bufferStart;
  size_t bufferEnd;
  std::vector<char> itsBuffer;
};

// ######################################################################
// Common BinaryArchive serialization functions

//! Saving for POD types to binary
template <class T>
CEREAL_HIDE_FUNCTION inline void
CEREAL_SAVE_FUNCTION_NAME(BinaryOutputArchive& ar, T const& t)
  requires(std::is_arithmetic_v<T>)
{
  ar.saveBinary(std::addressof(t), sizeof(t));
}

//! Loading for POD types from binary
template <class T>
CEREAL_HIDE_FUNCTION inline void
CEREAL_LOAD_FUNCTION_NAME(BinaryInputArchive& ar, T& t)
  requires(std::is_arithmetic_v<T>)
{
  ar.loadBinary(std::addressof(t), sizeof(t));
}

//! Serializing NVP types to binary
template <class Archive, class T>
CEREAL_HIDE_FUNCTION inline void
CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar, NameValuePair<T>& t)
    CEREAL_ARCHIVE_RESTRICT(BinaryInputArchive, BinaryOutputArchive) {
  ar(t.value);
}

//! Serializing SizeTags to binary
template <class Archive, class T>
CEREAL_HIDE_FUNCTION inline void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar,
                                                                SizeTag<T>& t)
    CEREAL_ARCHIVE_RESTRICT(BinaryInputArchive, BinaryOutputArchive) {
  ar(t.size);
}

//! Saving binary data
template <class T>
CEREAL_HIDE_FUNCTION inline void
CEREAL_SAVE_FUNCTION_NAME(BinaryOutputArchive& ar, BinaryData<T> const& bd) {
  ar.saveBinary(bd.data, static_cast<std::streamsize>(bd.size));
}

//! Loading binary data
template <class T>
CEREAL_HIDE_FUNCTION inline void
CEREAL_LOAD_FUNCTION_NAME(BinaryInputArchive& ar, BinaryData<T>& bd) {
  ar.loadBinary(bd.data, static_cast<std::streamsize>(bd.size));
}
} // namespace cereal

// register archives for polymorphic support
CEREAL_REGISTER_ARCHIVE(cereal::BinaryOutputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::BinaryInputArchive)

// tie input and output archives together
CEREAL_SETUP_ARCHIVE_TRAITS(cereal::BinaryInputArchive,
                            cereal::BinaryOutputArchive)

#endif // CEREAL_ARCHIVES_BINARY_HPP_
