ASYNC_SIMPLE_COPTS = select({
    "@com_github_async_simple//bazel/config:msvc_compiler": [
        "/std:c++20",
        "/await:strict",
        "/EHa"
    ],
    "//conditions:default": [
        "-std=c++20",
        "-D_GLIBCXX_USE_CXX11_ABI=1",
        "-Wno-deprecated-register",
        "-Wno-mismatched-new-delete",
        "-fPIC",
        "-Wall",
        "-Werror",
        "-D__STDC_LIMIT_MACROS",
        "-g",
    ],
})
