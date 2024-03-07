add_rules("mode.debug", "mode.release")

set_languages("c++20")

add_cxxflags("-stdlib=libstdc++")
add_requires("gtest")

target("std")
    set_kind("static")
    add_files("third_party_module/stdmodules/*.cppm")
    add_includedirs("third_party_module/stdmodules")
    set_languages("c++20")

target("asio")
    set_kind("static")
    add_files("third_party_module/asio/asio.cppm")
    add_includedirs("third_party_module/asio")
    set_languages("c++20")

target("async_simple")
    set_kind("static")
    add_files("async_simple_module/*.cppm",
              "async_simple_module/coro/*.cppm",
              "async_simple_module/executors/*.cppm",
              "async_simple_module/uthread/*.cppm",
              "async_simple_module/uthread/internal/*.cppm",
              "async_simple_module/uthread/internal/*.S",
              "async_simple_module/util/*.cppm")
    add_deps("std")
    set_languages("c++20")

target("async_simple_test")
    add_files("async_simple_module/test/*.cpp")
    add_deps("async_simple", "std")
    add_links("gtest", "gmock", "async_simple", "std", "pthread")

target("async_simple_coro_test")
    add_files("async_simple_module/test/coro/*.cpp", "async_simple_module/test/dotest.cpp")
    add_deps("async_simple", "std")
    add_links("gtest", "gmock", "async_simple", "std", "pthread")

target("async_simple_uthread_test")
    add_files("async_simple_module/test/uthread/*.cpp", "async_simple_module/test/dotest.cpp")
    add_deps("async_simple", "std")
    add_links("gtest", "gmock", "async_simple", "std", "pthread")

target("async_simple_util_test")
    add_files("async_simple_module/test/util/*.cpp", "async_simple_module/test/dotest.cpp")
    add_deps("async_simple", "std")
    add_links("gtest", "gmock", "async_simple", "std", "pthread")

