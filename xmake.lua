add_rules("mode.debug", "mode.release")

set_languages("c++20")

add_cxxflags("-stdlib=libstdc++")
add_requires("gtest")

target("async_simple")
    set_kind("static")
    add_files("modules/async_simple.cppm",
              "async_simple/executors/SimpleExecutor.cpp")
    add_includedirs(".")
    set_languages("c++20")

target("async_simple_coro_test")
    add_files("async_simple/coro/test/*.cpp", "async_simple/test/dotest.cpp")
    add_defines("USE_MODULES")
    add_includedirs(".")
    add_deps("async_simple")
    add_links("gtest", "gmock", "async_simple", "aio", "pthread")

target("async_simple_test")
    add_files("async_simple/test/*.cpp")
    add_defines("USE_MODULES")
    add_includedirs(".")
    add_deps("async_simple")
    add_links("gtest", "gmock", "async_simple", "aio", "pthread")

target("CountChar")
    add_files("demo_example/CountChar.cpp")
    add_defines("USE_MODULES")
    add_deps("async_simple")
    add_links("async_simple")

target("ReadFiles")
    add_files("demo_example/ReadFiles.cpp")
    add_defines("USE_MODULES")
    add_deps("async_simple")
    add_links("async_simple", "aio", "pthread")

