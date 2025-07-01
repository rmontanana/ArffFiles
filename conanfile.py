import re
from conan import ConanFile
from conan.tools.files import copy


class ArffFilesConan(ConanFile):
    name = "arff-files"
    version = "X.X.X"
    description = "Header-only library to read ARFF (Attribute-Relation File Format) files and return STL vectors with the data read."
    url = "https://github.com/rmontanana/ArffFiles"
    license = "MIT"
    homepage = "https://github.com/rmontanana/ArffFiles"
    topics = ("arff", "data-processing", "file-parsing", "header-only", "cpp17")
    no_copy_source = True
    exports_sources = "ArffFiles.hpp", "LICENSE", "README.md"
    package_type = "header-library"

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

    def package(self):
        # Copy header file to include directory
        copy(
            self,
            "*.hpp",
            src=self.source_folder,
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
        copy(
            self,
            "CMakeLists.txt",
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
