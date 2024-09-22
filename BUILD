# Copyright 2024 Daniel Koch

load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_python//python:pip.bzl", "compile_pip_requirements")

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

# bazel run //:requirements.update
compile_pip_requirements(
    name = "requirements",
    src = "requirements.txt",
    requirements_txt = "requirements_lock.txt",
)
