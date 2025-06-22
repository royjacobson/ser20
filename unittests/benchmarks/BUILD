load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "benchmark",
    srcs = [
        "benchmark.cpp",
    ],
    deps = [
        "//:ser20",
        "@google_benchmark//:benchmark",
    ],
)
