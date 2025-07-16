import re
from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class ArffFilesConan(ConanFile):
    name = "arff-files"
    version = "X.X.X"
    description = "Header-only library to read ARFF (Attribute-Relation \
        File Format) files and return STL vectors with the data read."
    url = "https://github.com/rmontanana/ArffFiles"
    license = "MIT"
    homepage = "https://github.com/rmontanana/ArffFiles"
    topics = ("arff", "data-processing", "file-parsing", "header-only", "cpp17")
    no_copy_source = True
    exports_sources = (
        "ArffFiles.hpp",
        "LICENSE",
        "README.md",
        "CMakeLists.txt",
        "config/*",
        "cmake/*",
    )
    package_type = "header-library"
    settings = "build_type", "compiler", "arch", "os"

    def init(self):
        # Read the CMakeLists.txt file to get the version
        with open("CMakeLists.txt", "r") as f:
            lines = f.readlines()
        for line in lines:
            if "VERSION" in line:
                # Extract the version number using regex
                match = re.search(r"VERSION\s+(\d+\.\d+\.\d+)", line)
                if match:
                    self.version = match.group(1)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.15]")
        self.test_requires("catch2/3.8.1")

    def layout(self):
        # Only use cmake_layout for conan packaging, not for development builds
        # This can be detected by checking if we're in a conan cache folder
        if (
            hasattr(self, "folders")
            and hasattr(self.folders, "base_build")
            and self.folders.base_build
            and ".conan2" in self.folders.base_build
        ):
            from conan.tools.cmake import cmake_layout

            cmake_layout(self)

    def generate(self):
        # Generate CMake toolchain file
        tc = CMakeToolchain(self)
        tc.generate()

        # Generate CMake dependencies file (needed for test requirements like catch2)
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        # Use CMake to generate the config file through existing config system
        from conan.tools.cmake import CMake

        cmake = CMake(self)
        # Configure with minimal options - just enough to generate the config file
        cmake.configure(
            build_script_folder=None,
            cli_args=["-DENABLE_TESTING=OFF", "-DCODE_COVERAGE=OFF"],
        )
        # No need to build anything, just configure to generate the config file

    def package(self):
        # Copy header file
        copy(
            self,
            "ArffFiles.hpp",
            src=self.source_folder,
            dst=self.package_folder,
            keep_path=False,
        )
        # Copy the generated config file from CMake build folder
        copy(
            self,
            "arffFiles_config.h",
            src=f"{self.build_folder}/configured_files/include",
            dst=self.package_folder,
            keep_path=False,
        )
        # Copy license and readme for package documentation
        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=self.package_folder,
            keep_path=False,
        )
        copy(
            self,
            "README.md",
            src=self.source_folder,
            dst=self.package_folder,
            keep_path=False,
        )

    def package_info(self):
        # Header-only library configuration
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        # Set include directory (header will be in package root)
        self.cpp_info.includedirs = ["."]
