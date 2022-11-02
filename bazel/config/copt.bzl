ASYNC_SIMPLE_COPTS = select({
    "//bazel/config:msvc_compiler": [
        "/std:c++20",
        "/await:strict",
        "/EHa"
    ],
    "//conditions:default": [
        "-std=c++20",
        "-D_GLIBCXX_USE_CXX11_ABI=1",
        "-Wno-deprecated-register",
        "-fPIC",
        "-Wall",
        "-Werror",
        "-D__STDC_LIMIT_MACROS",
        "-g",
        "-I.",
    ],
})
