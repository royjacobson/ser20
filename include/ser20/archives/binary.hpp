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
#ifndef SER20_ARCHIVES_BINARY_HPP_
#define SER20_ARCHIVES_BINARY_HPP_

#include "ser20/ser20.hpp"
#include <cassert>
#include <cstring>
#include <sstream>

namespace ser20 {
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
                    stream, or even cout!
      */
  BinaryOutputArchive(std::ostream& stream)
      : OutputArchive<BinaryOutputArchive, AllowEmptyClassElision>(this),
        itsStream(stream), bufferEnd(0), itsBuffer(bufferSize) {}

  ~BinaryOutputArchive() {
    if (bufferEnd) {
      flush();
    }
  }

  //! Writes size bytes of data to the output stream
  void saveBinary(const void* data, std::streamsize size) {
    auto as_char = reinterpret_cast<const char*>(data);
    if (4 * size > bufferSize) {
      flush();
      write(as_char, size);
    } else {
      if (bufferEnd + size > bufferSize) {
        flush();
      }
      std::memcpy(itsBuffer.data() + bufferEnd, as_char, size);
      bufferEnd += size;
    }
  }

  template <class... Args> SER20_HIDE_FUNCTION void process(Args&&... args) {
    (this->process(std::forward<Args>(args)), ...);
  }

  template <class T> SER20_HIDE_FUNCTION void process(T&& arg) {
    bool wasProcessing = isProcessing;
    if (!isProcessing) {
      isProcessing = true;
    }

    static_cast<OutputArchive*>(this)->process(std::forward<T>(arg));

    if (!wasProcessing) {
      flush();
      isProcessing = false;
    }
  }

private:
  void flush() {
    write(itsBuffer.data(), bufferEnd);
    bufferEnd = 0;
  }

  void write(const char* data, std::streamsize size) {
    auto const writtenSize = itsStream.rdbuf()->sputn(data, size);
    if (writtenSize != size)
      throw Exception("Failed to write " + std::to_string(size) +
                      " bytes to output stream! Wrote " +
                      std::to_string(writtenSize));
  }

  bool isProcessing = false;
  std::ostream& itsStream;

  static constexpr std::streamsize bufferSize = 0x1000;
  size_t bufferEnd;
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
  /*! @param stream The stream to output to.  Can be a stringstream, a file
             stream, or even cout!
    */
  BinaryInputArchive(std::istream& stream)
      : InputArchive<BinaryInputArchive, AllowEmptyClassElision>(this),
        itsStream(stream), bufferStart(0), bufferEnd(0), itsBuffer(bufferSize) {
  }

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

    if (4 * size > bufferSize) {
      load(data, size, size);
    } else {
      bufferStart = 0;
      bufferEnd = load(itsBuffer.data(), bufferSize, size);
      auto bytesRead = flush(data, size);
      (void)bytesRead;
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

  static constexpr std::streamsize bufferSize = 0x1000;
  size_t bufferStart;
  size_t bufferEnd;
  std::vector<char> itsBuffer;
};

// ######################################################################
// Common BinaryArchive serialization functions

//! Saving for POD types to binary
template <class T>
SER20_HIDE_FUNCTION inline void
SER20_SAVE_FUNCTION_NAME(BinaryOutputArchive& ar, T const& t)
  requires(std::is_arithmetic_v<T>)
{
  ar.saveBinary(std::addressof(t), sizeof(t));
}

//! Loading for POD types from binary
template <class T>
SER20_HIDE_FUNCTION inline void SER20_LOAD_FUNCTION_NAME(BinaryInputArchive& ar,
                                                         T& t)
  requires(std::is_arithmetic_v<T>)
{
  ar.loadBinary(std::addressof(t), sizeof(t));
}

//! Serializing NVP types to binary
template <class Archive, class T>
SER20_HIDE_FUNCTION inline void
SER20_SERIALIZE_FUNCTION_NAME(Archive& ar, NameValuePair<T>& t)
    SER20_ARCHIVE_RESTRICT(BinaryInputArchive, BinaryOutputArchive) {
  ar(t.value);
}

//! Serializing SizeTags to binary
template <class Archive, class T>
SER20_HIDE_FUNCTION inline void SER20_SERIALIZE_FUNCTION_NAME(Archive& ar,
                                                              SizeTag<T>& t)
    SER20_ARCHIVE_RESTRICT(BinaryInputArchive, BinaryOutputArchive) {
  ar(t.size);
}

//! Saving binary data
template <class T>
SER20_HIDE_FUNCTION inline void
SER20_SAVE_FUNCTION_NAME(BinaryOutputArchive& ar, BinaryData<T> const& bd) {
  ar.saveBinary(bd.data, static_cast<std::streamsize>(bd.size));
}

//! Loading binary data
template <class T>
SER20_HIDE_FUNCTION inline void SER20_LOAD_FUNCTION_NAME(BinaryInputArchive& ar,
                                                         BinaryData<T>& bd) {
  ar.loadBinary(bd.data, static_cast<std::streamsize>(bd.size));
}
} // namespace ser20

// register archives for polymorphic support
SER20_REGISTER_ARCHIVE(ser20::BinaryOutputArchive)
SER20_REGISTER_ARCHIVE(ser20::BinaryInputArchive)

// tie input and output archives together
SER20_SETUP_ARCHIVE_TRAITS(ser20::BinaryInputArchive,
                           ser20::BinaryOutputArchive)

#endif // SER20_ARCHIVES_BINARY_HPP_
