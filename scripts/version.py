"""Expose the firmware revision (`git describe`) as FIRMWARE_VERSION define."""

import subprocess

Import("env")  # noqa: F821 - provided by PlatformIO/SCons

try:
    version = subprocess.check_output(
        ["git", "describe", "--tags", "--always", "--dirty"],
        cwd=env.subst("$PROJECT_DIR"),  # noqa: F821
        text=True,
    ).strip()
except (OSError, subprocess.CalledProcessError):
    version = "unknown"

print(f"Firmware version: {version}")
env.Append(CPPDEFINES=[("FIRMWARE_VERSION", env.StringifyMacro(version))])  # noqa: F821
