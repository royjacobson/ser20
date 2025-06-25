load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "performance",
    srcs = [
        "performance.cpp",
    ],
    deps = [
        "//:ser20",
        "@google_benchmark//:benchmark",
        "@boost.serialization"
    ]
)
