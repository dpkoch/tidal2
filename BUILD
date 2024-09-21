load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "detail",
    hdrs = [
        "tidal/log_format.hh",
    ],
)

cc_library(
    name = "tidal",
    hdrs = ["tidal/tidal.hh"],
    visibility = ["//visibility:public"],
    deps = [
        ":detail",
        "@eigen",
    ],
)
