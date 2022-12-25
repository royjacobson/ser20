#include "ser20/ser20.hpp"
#include "ser20/archives/binary.hpp"
#include <sstream>
#include <memory>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "common.hpp"


TEST_SUITE_BEGIN("binary_archive");

TEST_CASE("binary_archive_dangling_stream")
{
    auto os = std::make_unique<std::ostringstream>();
    ser20::BinaryOutputArchive oar(*os);

    oar("TEST");
    os.reset();
    // Check that this doesn't crash the archive.
}

TEST_SUITE_END();
