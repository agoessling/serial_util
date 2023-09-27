load("crc_tables.bzl", "crc_table", "crc_repo")

cc_library(
    name = "c_crc",
    srcs = ["c_crc.c"],
    hdrs = ["c_crc.h"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "c_crc.so",
    srcs = [
        "c_crc.c",
        "c_crc.h",
    ],
    linkshared = True,
    visibility = ["//visibility:private"],
)

cc_test(
    name = "test_c_crc",
    srcs = ["test_c_crc.c"],
    visibility = ["//visibility:private"],
    deps = [
        ":all_crcs",
        ":c_crc",
        "@unity",
    ],
)

cc_library(
    name = "cc_crc",
    hdrs = ["cc_crc.h"],
    visibility = ["//visibility:private"],
    deps = [
        ":c_crc",
    ],
)

cc_test(
    name = "test_cc_crc",
    srcs = ["test_cc_crc.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":all_crcs",
        ":cc_crc",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

py_library(
    name = "py_crc",
    srcs = ["py_crc.py"],
    data = [
        ":c_crc.so",
        ":all_crcs.so",
    ],
    visibility = ["//visibility:public"],
)

py_test(
    name = "test_py_crc",
    srcs = ["test_py_crc.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":py_crc",
    ],
)

py_binary(
    name = "gen_crc_table",
    srcs = ["gen_crc_table.py"],
    visibility = ["//visibility:public"],
)

crc_repo(
    visibility = ["//visibility:public"],
)
