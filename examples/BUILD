# Copyright 2024 Daniel Koch

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_python//python:defs.bzl", "py_binary")

cc_binary(
    name = "write_log",
    srcs = ["write_log.cc"],
    deps = [
        "//:tidal",
        "@eigen",
    ],
)

py_binary(
    name = "read_log",
    srcs = ["read_log.py"],
    deps = [
        "//:tidal_parser",
        "@pypi//debugpy",
    ],
)
