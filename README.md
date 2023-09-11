# serial_util

Utilities useful for sending and receiving serial data.

## Usage

### WORKSPACE

To incorporate `serial_util` into your project copy the following into your `WORKSPACE` file.

```Starlark
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "serial_util",
    # See release page for latest version url and sha.
)
```
