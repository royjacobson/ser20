/*! \file macros.hpp
    \brief Preprocessor macros that can customise the ser20 library

    By default, ser20 looks for serialization functions with very
    specific names, that is: serialize, load, save, load_minimal,
    or save_minimal.

    This file allows an advanced user to change these names to conform
    to some other style or preference.  This is implemented using
    preprocessor macros.

    As a result of this, in internal ser20 code you will see macros
    used for these function names.  In user code, you should name
    the functions like you normally would and not use the macros
    to improve readability.
    \ingroup utility */
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

#ifndef SER20_MACROS_HPP_
#define SER20_MACROS_HPP_

#ifdef _MSVC_LANG
#if _MSVC_LANG <= 201703L
#error "Ser20 requires at least C++20!"
#endif
#else
#if __cplusplus <= 201703L
#error "Ser20 requires at least C++20!"
#endif
#endif

#ifndef SER20_THREAD_SAFE
//! Whether ser20 should be compiled for a threaded environment
/*! This macro causes ser20 to use mutexes to control access to
    global internal state in a thread safe manner.

    Note that even with this enabled you must still ensure that
    archives are accessed by only one thread at a time; it is safe
    to use multiple archives in parallel, but not to access one archive
    from many places simultaneously. */
#define SER20_THREAD_SAFE 0
#endif // SER20_THREAD_SAFE

#ifndef SER20_SIZE_TYPE
//! Determines the data type used for size_type
/*! ser20 uses size_type to ensure that the serialized size of
    dynamic containers is compatible across different architectures
    (e.g. 32 vs 64 bit), which may use different underlying types for
    std::size_t.

    More information can be found in ser20/details/helpers.hpp.

    If you choose to modify this type, ensure that you use a fixed
    size type (e.g. uint32_t). */
#define SER20_SIZE_TYPE uint64_t
#endif // SER20_SIZE_TYPE

// ######################################################################
#ifndef SER20_SERIALIZE_FUNCTION_NAME
//! The serialization/deserialization function name to search for.
/*! You can define @c SER20_SERIALIZE_FUNCTION_NAME to be different assuming
    you do so before this file is included. */
#define SER20_SERIALIZE_FUNCTION_NAME serialize
#endif // SER20_SERIALIZE_FUNCTION_NAME

#ifndef SER20_LOAD_FUNCTION_NAME
//! The deserialization (load) function name to search for.
/*! You can define @c SER20_LOAD_FUNCTION_NAME to be different assuming you do
   so before this file is included. */
#define SER20_LOAD_FUNCTION_NAME load
#endif // SER20_LOAD_FUNCTION_NAME

#ifndef SER20_SAVE_FUNCTION_NAME
//! The serialization (save) function name to search for.
/*! You can define @c SER20_SAVE_FUNCTION_NAME to be different assuming you do
   so before this file is included. */
#define SER20_SAVE_FUNCTION_NAME save
#endif // SER20_SAVE_FUNCTION_NAME

#ifndef SER20_LOAD_MINIMAL_FUNCTION_NAME
//! The deserialization (load_minimal) function name to search for.
/*! You can define @c SER20_LOAD_MINIMAL_FUNCTION_NAME to be different assuming
   you do so before this file is included. */
#define SER20_LOAD_MINIMAL_FUNCTION_NAME load_minimal
#endif // SER20_LOAD_MINIMAL_FUNCTION_NAME

#ifndef SER20_SAVE_MINIMAL_FUNCTION_NAME
//! The serialization (save_minimal) function name to search for.
/*! You can define @c SER20_SAVE_MINIMAL_FUNCTION_NAME to be different assuming
   you do so before this file is included. */
#define SER20_SAVE_MINIMAL_FUNCTION_NAME save_minimal
#endif // SER20_SAVE_MINIMAL_FUNCTION_NAME

// ######################################################################
//! Defines the SER20_ALIGNOF macro to use instead of alignof
#if defined(_MSC_VER) && _MSC_VER < 1900
#define SER20_ALIGNOF __alignof
#else // not MSVC 2013 or older
#define SER20_ALIGNOF alignof
#endif // end MSVC check

#if !defined(SER20_NODEBUG) && defined(__has_attribute)
#  if __has_attribute(__nodebug__)
#    define SER20_NODEBUG __attribute__((__nodebug__))
#  endif
#endif
#ifndef SER20_NODEBUG
#define SER20_NODEBUG
#endif

#if defined(__has_attribute)
#  if __has_attribute(__always_inline__)
#    define SER20_FORCE_INLINE __attribute__((__always_inline__))
#  endif
#endif
#if defined(_MSC_VER) && !defined(SER20_FORCE_INLINE)
#  define SER20_FORCE_INLINE [[msvc::forceinline]]
#endif
#ifndef SER20_FORCE_INLINE
#define SER20_FORCE_INLINE
#endif

#ifndef SER20_HIDE_FUNCTION
#define SER20_HIDE_FUNCTION SER20_NODEBUG SER20_FORCE_INLINE
#endif

#endif // SER20_MACROS_HPP_
