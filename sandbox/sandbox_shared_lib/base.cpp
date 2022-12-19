#ifndef SER20_DLL_USE
#define SER20_DLL_MAKE
#endif
#include "base.hpp"

template void Base::serialize<ser20::XMLOutputArchive>
    ( ser20::XMLOutputArchive & ar, std::uint32_t const version );
template void Base::serialize<ser20::XMLInputArchive>
    ( ser20::XMLInputArchive & ar, std::uint32_t const version );
