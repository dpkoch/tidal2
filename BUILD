# Copyright 2024 Daniel Koch

load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_python//python:defs.bzl", "py_library")
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

py_library(
    name = "parser_detail",
    srcs = ["tidal_parser/detail.py"],
    deps = [
        "@pypi//numpy",
    ],
)

py_library(
    name = "tidal_parser_impl",
    srcs = ["tidal_parser/tidal_parser.py"],
    deps = [
        ":parser_detail",
        "@pypi//numpy",
    ],
)

py_library(
    name = "tidal_parser",
    srcs = ["tidal_parser/__init__.py"],
    visibility = ["//visibility:public"],
    deps = [":tidal_parser_impl"],
)

# bazel run //:requirements.update
compile_pip_requirements(
    name = "requirements",
    src = "requirements.txt",
    requirements_txt = "requirements_lock.txt",
)
