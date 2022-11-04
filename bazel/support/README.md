## Bazel support
Make sure that Bazel is installed on your system.


## Using {async_simple} as a dependency
The following minimal example shows how to use {async_simple} as a dependency within a Bazel project.

The following file structure is assumed:
```bash
hello-async_simple/
├── BUILD.bazel
├── src
│   └── hello.cc
└── WORKSPACE.bazel
```

*hello.cc:*
```cpp
#include "async_simple/uthread/Uthread.h"
#include <iostream>

int main() {
    async_simple::uthread::Uthread uth({}, []() {
        std::cout << "Hello async_simple!" << std::endl;
    });
}
```
The expected output of this example is `Hello async_simple!`.

*WORKSPACE.bazel*:
```python
# Use git_repository
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

ASYNC_SIMPLE_COMMID_ID = ""

git_repository(
    name = "com_github_async_simple",
    commit = ASYNC_SIMPLE_COMMID_ID,
    remote = "https://github.com/alibaba/async_simple.git",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
BAZEL_SKYLIB_VERSION = "1.1.1"  # 2021-09-27
BAZEL_SKYLIB_SHA256 = "c6966ec828da198c5d9adbaa94c05e3a1c7f21bd012a0b29ba8ddbccb2c93b0d"

http_archive(
    name = "bazel_skylib",
    sha256 = BAZEL_SKYLIB_SHA256,
    urls = [
        "https://github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
    ],
)

# Use http_archive
AYSNC_SIMPLE_VERSION = ""
ASYNC_SIMPLE_SHA256 = ""

http_archive(
    name = "com_github_async_simple",
    sha256 = ASYNC_SIMPLE_SHA256,
    urls = ["https://github.com/alibaba/async_simple/archive/refs/tags/{version}.zip".format(version = AYSNC_SIMPLE_VERSION)],
    strip_prefix = "async_simple-{version}".format(version = AYSNC_SIMPLE_VERSION),
)
```

*BUILD.bazel*:
```python
cc_binary(
    name = "hello",
    srcs = ["src/hello.cc"],
    deps = ["@com_github_async_simple//:async_simple"],
    copts = ["-std=c++20"],
)
```

The *BUILD* file defines a binary named `Demo` that has a dependency to {async_simple}.  

To execute the binary you can run `bazel run :hello`.
