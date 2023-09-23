workspace(name = "serial_util")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Google test
http_archive(
    name = "gtest",
    sha256 = "353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a",
    strip_prefix = "googletest-release-1.11.0",
    url = "https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip",
)

# unity
http_archive(
    name = "unity",
    build_file = "//third_party:unity.BUILD",
    sha256 = "af00e4ecfcb7546cc8e6d39fe3770fa100b19e59ff63f7b6dcbd3010eac9f35f",
    strip_prefix = "Unity-2.5.1",
    url = "https://github.com/ThrowTheSwitch/Unity/archive/v2.5.1.zip",
)

# bazel_lint
http_archive(
    name = "bazel_lint",
    sha256 = "85b8cab2998fc7ce32294d6473276ba70eea06b0eef4bce47de5e45499e7096f",
    strip_prefix = "bazel_lint-0.1.1",
    url = "https://github.com/agoessling/bazel_lint/archive/v0.1.1.zip",
)

load("@bazel_lint//bazel_lint:bazel_lint_first_level_deps.bzl", "bazel_lint_first_level_deps")

bazel_lint_first_level_deps()

load("@bazel_lint//bazel_lint:bazel_lint_second_level_deps.bzl", "bazel_lint_second_level_deps")

bazel_lint_second_level_deps()

http_archive(
    name = "bazel_skylib",
    sha256 = "66ffd9315665bfaafc96b52278f57c7e2dd09f5ede279ea6d39b2be471e7e3aa",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()
