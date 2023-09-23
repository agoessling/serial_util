def crc_table(name, table_name, bits, polynomial, lsb_first, **kwargs):
    args = [
        "-b",
        str(bits),
        "-p",
        str(polynomial),
        "--header",
        "$(location :{}.h)".format(name),
        "--source",
        "$(location :{}.c)".format(name),
        "--name",
        table_name,
    ]

    if lsb_first:
        args += ["--lsb_first"]

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
        **kwargs
    )
