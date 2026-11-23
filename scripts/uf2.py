"""Convert firmware.hex to firmware.uf2 for drag and drop flashing via the UF2 bootloader."""

from pathlib import Path

Import("env")  # noqa: F821 - provided by PlatformIO/SCons

# family ID of the Adafruit nRF52840 bootloader (used by the nice!nano)
UF2_FAMILY_ID = "0xADA52840"

framework_dir = env.PioPlatform().get_package_dir("framework-arduinoadafruitnrf52")  # noqa: F821
uf2conv = Path(framework_dir) / "tools" / "uf2conv" / "uf2conv.py"

env.AddPostAction(  # noqa: F821
    "$BUILD_DIR/${PROGNAME}.hex",
    env.VerboseAction(  # noqa: F821
        f'"$PYTHONEXE" -W ignore::SyntaxWarning "{uf2conv}" --convert --family {UF2_FAMILY_ID} '
        '--output "$BUILD_DIR/${PROGNAME}.uf2" "$BUILD_DIR/${PROGNAME}.hex"',
        "Building $BUILD_DIR/${PROGNAME}.uf2",
    ),
)
