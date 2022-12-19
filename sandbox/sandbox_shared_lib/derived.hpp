#pragma once
#include "base.hpp"
class Derived : public Base
{
  public:
    virtual ~Derived() {}

  private:
    friend class ser20::access;
    template <class Archive>
    void serialize(Archive & ar, std::uint32_t const)
    {
      ar(ser20::base_class<Base>(this));
    }
};

extern template DECLSPECIFIER void Derived::serialize<ser20::XMLOutputArchive>
    ( ser20::XMLOutputArchive & ar, std::uint32_t const version );
extern template DECLSPECIFIER void Derived::serialize<ser20::XMLInputArchive>
    ( ser20::XMLInputArchive & ar, std::uint32_t const version );

SER20_REGISTER_TYPE(Derived)
