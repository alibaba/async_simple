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

# Use http_archive
AYSNC_SIMPLE_VERSION = ""
ASYNC_SIMPLE_SHA256 = ""

http_archive(
    name = "com_github_async_simple",
    sha256 = ASYNC_SIMPLE_SHA256,
    urls = ["https://github.com/alibaba/async_simple/archive/refs/tags/{version}.zip".format(version = AYSNC_SIMPLE_VERSION)],
    strip_prefix = "async_simple-{version}".format(version = AYSNC_SIMPLE_VERSION),
)

# Load async_simple async_simple_dependencies
load("@com_github_async_simple//bazel/config:deps.bzl", "async_simple_dependencies")
async_simple_dependencies()
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

The *BUILD* file defines a binary named `hello` that has a dependency to {async_simple}.  

To execute the binary you can run `bazel run :hello`.
