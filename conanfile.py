from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("boost/1.83.0", 
            options={
                "header_only": True,
                "without_test": True,
                "without_program_options": True,
                "without_graph": True,
                "without_serialization": True,
                "without_wave": True,
                "without_log": False,
                "without_random": False
            }
        )
        self.requires("cpr/1.11.2")
        self.requires("drogon/1.9.10")
        self.requires("nlohmann_json/3.12.0")
        self.requires("spdlog/1.11.0")
        self.requires("openssl/3.4.1")
        self.requires("libiconv/1.18")

    def layout(self):
        cmake_layout(self)
