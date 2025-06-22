load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "ser20",
    srcs = [
        "src/ser20.cpp",
        "src/polymorphic_impl.cpp",
    ],
    hdrs = [
        "include/ser20/types/concepts/pair_associative_container.hpp",
    ] + glob([
        "include/ser20/*.hpp",
        "include/ser20/external/*.hpp",
        "include/ser20/external/rapidjson/*.h",
        "include/ser20/external/rapidjson/error/*.h",
        "include/ser20/external/rapidjson/internal/*.h",
        "include/ser20/external/rapidjson/msinttypes/*.h",
        "include/ser20/external/rapidxml/*.hpp",
        "include/ser20/archives/*.hpp",
        "include/ser20/details/*.hpp",
        "include/ser20/types/*.hpp",
    ]),
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
)
