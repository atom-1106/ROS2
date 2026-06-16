
from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import mkdir, copy
from os import path, remove
from shutil import rmtree


class CreateVerity(ConanFile):
    settings = "os", "build_type", "arch"
    STAGING_DIR = "staging"
    OUTPUT_DIR = "verity"

    def validate(self):
        if not self.settings.arch == "armv8":
            raise ConanInvalidConfiguration(
                "Creating verities only makes sense for ARM binaries")

    def layout(self):
        self.folders.build = "build"
        self.folders.generators = "build/generators"

    def create_verity(self, verityName: str):
        # Create the verity
        copy(self,
             pattern="app_init.sh",
             src=self.folders.source_folder,
             dst=self.STAGING_DIR)

        mkdir(self, self.OUTPUT_DIR)

        TABLE_FILE = path.join(self.OUTPUT_DIR, verityName + ".table")
        EXTERNAL_PATH = path.join(
            self.folders.source_folder, "../../../external")
        CREATE_VERITY_SCRIPT = path.join(
            EXTERNAL_PATH, "veritysetup/generate_verity_and_table_linux3x.sh")
        CREATE_VERITY_COMMAND = [CREATE_VERITY_SCRIPT,
                                 verityName, self.STAGING_DIR, self.OUTPUT_DIR]
        self.run(" ".join(CREATE_VERITY_COMMAND), env="conanrun")

        CREATE_VERITY_VERIFY_SCRIPT = path.join(
            EXTERNAL_PATH, "veritysetup/sign_table_to_verity_verify.sh")
        SIGNING_CERT = path.join(
            EXTERNAL_PATH, "core_telematics_bin/dev_signing/signing-cert.pem")
        SIGNING_KEY = path.join(
            EXTERNAL_PATH, "core_telematics_bin/dev_signing/signing-key.pem")
        SIGNING_PASSPHRASE = "notproduction"
        CREATE_VERITY_VERIFY_COMMAND = [CREATE_VERITY_VERIFY_SCRIPT, TABLE_FILE,
                                        self.OUTPUT_DIR, SIGNING_CERT, SIGNING_KEY, SIGNING_PASSPHRASE]
        self.run(" ".join(CREATE_VERITY_VERIFY_COMMAND), env="conanrun")

        # Cleanup
        remove(TABLE_FILE)
        # rmtree(path.join(self.folders.build_folder, self.STAGING_DIR))

    def package(self):
        copy(self,
             pattern="*.verity*",
             src=self.OUTPUT_DIR, dst=self.package_folder)
