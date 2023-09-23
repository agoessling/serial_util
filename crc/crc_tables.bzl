load("@bazel_skylib//rules:write_file.bzl", "write_file")

def crc_table(name, crc_name, bits, polynomial, initial_crc, final_xor, lsb_first, **kwargs):
    args = [
        "-b",
        str(bits),
        "-p",
        str(polynomial),
        "-i",
        str(initial_crc),
        "-x",
        str(final_xor),
        "--header",
        "$(location :{}.h)".format(name),
        "--source",
        "$(location :{}.c)".format(name),
        "--name",
        crc_name,
    ]

    if lsb_first:
        args.append("--lsb_first")

    native.genrule(
        name = name + "_gen",
        outs = [
            name + ".c",
            name + ".h",
        ],
        cmd = "$(execpath @//crc:gen_crc_table) {}".format(" ".join(args)),
        tools = ["@//crc:gen_crc_table"],
        visibility = ["//visibility:private"],
    )

    native.cc_library(
        name = name,
        srcs = [name + ".c"],
        hdrs = [name + ".h"],
        deps = [
            "@//crc:c_crc",
        ],
        **kwargs
    )

def crc_repo(**kwargs):
    crcs = [
        ("crc_8_darc", "kCrc8Darc", 8, 0x39, 0x00, 0x00, True),
        ("crc_8_i_code", "kCrc8ICode", 8, 0x1D, 0xFD, 0x00, False),
        ("crc_16_kermit", "kCrc16Kermit", 16, 0x1021, 0x0000, 0x0000, True),
        ("crc_16_ccitt_false", "kCrc16CcittFalse", 16, 0x1021, 0xFFFF, 0x0000, False),
        ("crc_32", "kCrc32", 32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, True),
        ("crc_32_mpeg_2", "kCrc32Mpeg2", 32, 0x04C11DB7, 0xFFFFFFFF, 0x00000000, False),
    ]

    for crc in crcs:
        crc_table(*crc, **kwargs)

    all_targets = [":" + x[0] for x in crcs]
    all_srcs = [x[0] + ".c" for x in crcs]
    all_hdrs = [x[0] + ".h" for x in crcs]

    lines = [
        "#pragma once",
        "",
    ]
    lines += ["#include \"crc/{}\"".format(h) for h in all_hdrs]

    write_file(
        name = "all_crcs_gen",
        out = "all_crcs.h",
        content = lines,
        visibility = ["//visibility:private"],
    )

    native.cc_library(
        name = "all_crcs",
        hdrs = ["all_crcs.h"],
        deps = all_targets + ["@//crc:c_crc"],
        **kwargs
    )

    native.cc_binary(
        name = "all_crcs.so",
        srcs = all_srcs,
        linkshared = True,
        deps = ["@//crc:c_crc"],
        **kwargs
    )
