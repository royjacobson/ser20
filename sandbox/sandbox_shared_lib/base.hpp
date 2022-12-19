#pragma once

#include <memory>
#include <ser20/ser20.hpp>
#include <ser20/archives/xml.hpp>
#include <ser20/types/polymorphic.hpp>

#if defined (_WINDLL)
#define DECLSPECIFIER __declspec(dllexport)
#elif defined(MSC_VER)
#define DECLSPECIFIER __declspec(dllimport)
#else
#define DECLSPECIFIER
#endif

int doit();

class VersionTest
{
  public:
    int x;
    template <class Archive>
    void serialize( Archive & ar, const std::uint32_t /* version */ )
    { ar( x ); }
};

class Base
{
  public:
    friend class ser20::access;

    template < class Archive >
    void serialize(Archive &, std::uint32_t const) {}
    virtual ~Base() {}
};

extern template DECLSPECIFIER void Base::serialize<ser20::XMLInputArchive>
( ser20::XMLInputArchive & ar, std::uint32_t const version );

extern template DECLSPECIFIER void Base::serialize<ser20::XMLOutputArchive>
( ser20::XMLOutputArchive & ar, std::uint32_t const version );

SER20_CLASS_VERSION(VersionTest, 1)
