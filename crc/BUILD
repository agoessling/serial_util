load("crc_tables.bzl", "crc_table")

cc_library(
    name = "c_crc",
    srcs = ["c_crc.c"],
    hdrs = ["c_crc.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test_c_crc",
    srcs = ["test_c_crc.c"],
    visibility = ["//visibility:public"],
    deps = [
        ":c_crc",
        "@unity",
    ],
)

py_binary(
    name = "gen_crc_table",
    srcs = ["gen_crc_table.py"],
    visibility = ["//visibility:public"],
)

crc_table(
    name = "crc_16_kermit",
    table_name = "kCrc16Kermit",
    bits = 16,
    polynomial = 0x1021,
    lsb_first = True,
)
