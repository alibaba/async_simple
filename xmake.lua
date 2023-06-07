add_rules("mode.debug", "mode.release")

set_languages("c++20")

add_requires("gtest")
if is_plat("linux") then
    add_requires("libaio")
    add_syslinks("pthread")
    add_cxxflags("-stdlib=libstdc++")
else
    add_defines("ASYNC_SIMPLE_HAS_NOT_AIO", "ASYNC_SIMPLE_MODULES_HAS_NOT_UTHREAD")
end

target("async_simple")
    set_kind("static")
    add_files("modules/async_simple.cppm")
    add_files("async_simple/executors/SimpleExecutor.cpp")
    add_includedirs(".")
    add_packages("libaio", {public = true})

target("async_simple_coro_test")
    add_deps("async_simple")
    add_files("async_simple/coro/test/*.cpp", "async_simple/test/dotest.cpp")
    add_defines("USE_MODULES")
    add_includedirs(".")
    add_packages("gtest")
    if is_plat("windows") then
        remove_files("async_simple/coro/test/LazyTest.cpp")
    end

target("async_simple_test")
    add_deps("async_simple")
    add_files("async_simple/test/*.cpp")
    add_defines("USE_MODULES")
    add_includedirs(".")
    add_packages("gtest")
    if is_plat("windows") then
        remove_files("async_simple/test/FutureTest.cpp")
    end

target("CountChar")
    add_deps("async_simple")
    add_files("demo_example/CountChar.cpp")
    add_defines("USE_MODULES")
    if is_plat("windows") then
        set_enabled(false)
    end

target("ReadFiles")
    add_deps("async_simple")
    add_files("demo_example/ReadFiles.cpp")
    add_defines("USE_MODULES")
    if is_plat("windows") then
        set_enabled(false)
    end

