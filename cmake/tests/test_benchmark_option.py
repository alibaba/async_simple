"""Configure-only regression tests; run with Python 3, CMake and Ninja."""

from pathlib import Path
import re
import subprocess
import tempfile
import unittest


SOURCE = Path(__file__).resolve().parents[2]


class BenchmarkOptionTest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="async-simple-benchmark-")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        include = self.directory / "include"
        (include / "benchmark").mkdir(parents=True)
        (include / "benchmark" / "benchmark.h").touch()
        library = self.directory / "libbenchmark.a"
        library.touch()
        # Only configuration and target generation use these placeholders.
        self.dependencies = {
            "BENCHMARK_INCLUDE_DIR": include.as_posix(),
            "BENCHMARK_LIBRARIES": library.as_posix(),
        }

    def run_command(self, command):
        result = subprocess.run(
            command, capture_output=True, text=True, timeout=120
        )
        self.assertEqual(
            result.returncode, 0,
            "{}\n{}\n{}".format(command, result.stdout, result.stderr),
        )
        return result

    def configure(self, name, discovery, target, **variables):
        build = self.directory / name
        command = [
            "cmake", "-S", str(SOURCE), "-B", str(build), "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DASYNC_SIMPLE_ENABLE_TESTS=OFF",
            "-DASYNC_SIMPLE_BUILD_DEMO_EXAMPLE=OFF",
            "-DASYNC_SIMPLE_ENABLE_ASAN=OFF",
            "-DASYNC_SIMPLE_DISABLE_AIO=ON",
            "-DASYNC_SIMPLE_BUILD_MODULES=OFF",
            "--trace-expand",
            "--trace-source=" + str(SOURCE / "CMakeLists.txt"),
        ]
        command.extend("-D{}={}".format(key, value)
                       for key, value in variables.items())
        result = self.run_command(command)
        self.assertEqual(
            bool(re.search(r"\bfind_package\(Benchmark(?:\s|\))", result.stderr)),
            discovery,
            result.stderr,
        )
        self.assertEqual(
            bool(re.search(r"\badd_subdirectory\(benchmarks(?:\s|\))", result.stderr)),
            target,
            result.stderr,
        )
        self.assertEqual("-- Benchmark found." in result.stdout, target)
        graph = self.run_command(
            ["ninja", "-C", str(build), "-t", "targets", "all"]
        ).stdout
        targets = {line.split(":", 1)[0] for line in graph.splitlines()}
        self.assertEqual(
            bool(targets & {"benchmarking", "benchmarks/benchmarking"}),
            target,
            graph,
        )
        cache = {}
        for line in (build / "CMakeCache.txt").read_text().splitlines():
            if line and not line.startswith(("#", "//")) and "=" in line:
                key, value = line.split("=", 1)
                cache[key.split(":", 1)[0]] = value
        return cache

    def test_fresh_off(self):
        self.configure("off", False, False, ASYNC_SIMPLE_ENABLE_BENCHMARKS="OFF")

    def test_fresh_off_with_populated_variables(self):
        cache = self.configure(
            "off-populated", False, False,
            ASYNC_SIMPLE_ENABLE_BENCHMARKS="OFF", **self.dependencies
        )
        for key, value in self.dependencies.items():
            self.assertEqual(cache[key], value)

    def test_on_off_on_retains_cache(self):
        self.configure(
            "reuse", True, True,
            ASYNC_SIMPLE_ENABLE_BENCHMARKS="ON", **self.dependencies
        )
        cache = self.configure(
            "reuse", False, False, ASYNC_SIMPLE_ENABLE_BENCHMARKS="OFF"
        )
        for key, value in self.dependencies.items():
            self.assertEqual(cache[key], value)
        self.configure(
            "reuse", True, True, ASYNC_SIMPLE_ENABLE_BENCHMARKS="ON"
        )

    def test_default_off(self):
        for populated in (False, True):
            with self.subTest(populated=populated):
                variables = self.dependencies if populated else {}
                cache = self.configure(
                    "default-" + str(populated), False, False, **variables
                )
                self.assertEqual(cache["ASYNC_SIMPLE_ENABLE_BENCHMARKS"], "OFF")
                for key, value in variables.items():
                    self.assertEqual(cache[key], value)

    def test_fresh_on(self):
        cache = self.configure(
            "on", True, True,
            ASYNC_SIMPLE_ENABLE_BENCHMARKS="ON", **self.dependencies
        )
        self.assertEqual(cache["ASYNC_SIMPLE_ENABLE_BENCHMARKS"], "ON")

    def test_optional_package_unavailable(self):
        # Simulate unavailable discovery independently of installed packages.
        self.configure(
            "unavailable", True, False,
            ASYNC_SIMPLE_ENABLE_BENCHMARKS="ON",
            CMAKE_DISABLE_FIND_PACKAGE_Benchmark="TRUE",
        )

    def test_on_requires_both_dependency_variables(self):
        for missing in self.dependencies:
            with self.subTest(missing=missing):
                variables = dict(self.dependencies)
                variables[missing] = missing + "-NOTFOUND"
                self.configure(
                    "missing-" + missing, True, False,
                    ASYNC_SIMPLE_ENABLE_BENCHMARKS="ON",
                    CMAKE_DISABLE_FIND_PACKAGE_Benchmark="TRUE",
                    **variables
                )


if __name__ == "__main__":
    unittest.main()
