cc_library(
    name = "c_cobs",
    srcs = ["c_cobs.c"],
    hdrs = ["c_cobs.h"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "c_cobs.so",
    srcs = [
        "c_cobs.c",
        "c_cobs.h",
    ],
    linkshared = True,
    visibility = ["//visibility:private"],
)

cc_test(
    name = "test_c_cobs",
    srcs = ["test_c_cobs.c"],
    visibility = ["//visibility:public"],
    deps = [
        ":c_cobs",
        "@unity",
    ],
)

cc_library(
    name = "cc_cobs",
    hdrs = ["cc_cobs.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":c_cobs",
    ],
)

cc_test(
    name = "test_cc_cobs",
    srcs = ["test_cc_cobs.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":cc_cobs",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

py_library(
    name = "py_cobs",
    srcs = ["py_cobs.py"],
    data = [":c_cobs.so"],
    visibility = ["//visibility:public"],
)

py_test(
    name = "test_py_cobs",
    srcs = ["test_py_cobs.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":py_cobs",
    ],
)
