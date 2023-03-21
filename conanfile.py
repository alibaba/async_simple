from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy, get
import os.path as osp


class AsyncSimpleConan(ConanFile):
    name = "async_simple"
    version = "1.0.0"
    url = "https://github.com/alibaba/async_simple"
    description = "Simple, light-weight and easy-to-use asynchronous components"
    topics = "coroutine"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "header_only": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "header_only": True
    }
    exports_sources = "CMakeLists.txt", "cmake/*", "async_simple/*",

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.header_only:
            del self.options.shared
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        if not self.options.header_only:
            tc = CMakeToolchain(self)
            tc.variables["ASYNC_SIMPLE_BUILD_DEMO_EXAMPLE"] = False
            tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        if self.options.header_only:
            return
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", dst=osp.join(self.package_folder, "licenses"), src=self.source_folder)
        if self.options.header_only:
            excludes = None
            if self.settings.os in ["Macos"]:
                excludes = 'uthread', 'test'
            copy(self, pattern="*.h",
                 dst=osp.join(self.package_folder, "include/async_simple"),
                 src=osp.join(self.source_folder, "async_simple"),
                 excludes=excludes)
        else:
            cmake = CMake(self)
            cmake.install()

    def package_id(self):
        if self.info.options.header_only:
            self.info.clear()

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "async_simple"
        self.cpp_info.components["async_simple"].names["cmake_find_package"] = "async_simple"
        if self.options.header_only:
            self.cpp_info.components["async_simple"].set_property("cmake_target_name",
                                                                  f"async_simple::async_simple_header_only")
        else:
            self.cpp_info.components["async_simple"].set_property("cmake_target_name", f"async_simple::async_simple")

        if self.settings.os in ["Macos"]:
            self.cpp_info.components["async_simple"].defines = ["ASYNC_SIMPLE_HAS_NOT_AIO=1"]
