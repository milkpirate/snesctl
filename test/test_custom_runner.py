"""PlatformIO test runner for Catch2 (`test_framework = custom`).

Fetches the pinned Catch2 amalgamated sources (compiled by `catch2_main.cpp`) and
reports the test cases by parsing the output of Catch2's TeamCity reporter.
"""

import re
import urllib.request
from pathlib import Path

from platformio.public import TestCase, TestCaseSource, TestRunnerBase, TestStatus

CATCH2_VERSION = "v3.16.1"
CATCH2_URL = "https://raw.githubusercontent.com/catchorg/Catch2/{version}/extras/{file}"
CATCH2_FILES = ("catch_amalgamated.hpp", "catch_amalgamated.cpp")

TEAMCITY_RE = re.compile(r"##teamcity\[(?P<event>\w+)(?P<attrs>.*)\]\s*$")
TEAMCITY_ATTR_RE = re.compile(r"(\w+)='((?:\|.|[^|'])*)'")
TEAMCITY_UNESCAPE = {"|n": "\n", "|r": "\r", "|'": "'", "|[": "[", "|]": "]", "||": "|"}
SOURCE_RE = re.compile(r"^(?P<file>.+?):(?P<line>\d+)", re.MULTILINE)


def teamcity_unescape(value):
    return re.sub(r"\|.", lambda m: TEAMCITY_UNESCAPE.get(m.group(0), m.group(0)), value)


class CustomTestRunner(TestRunnerBase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._failures = {}

    def configure_build_env(self, env):
        workspace_dir = self.project_config.get("platformio", "workspace_dir")
        catch2_dir = Path(workspace_dir) / "catch2" / CATCH2_VERSION
        catch2_dir.mkdir(parents=True, exist_ok=True)
        for file in CATCH2_FILES:
            target = catch2_dir / file
            if not target.exists():
                url = CATCH2_URL.format(version=CATCH2_VERSION, file=file)
                print(f"Downloading {url}")
                urllib.request.urlretrieve(url, target)

        env.Append(
            CPPPATH=[str(catch2_dir)],
            CPPDEFINES=[("CATCH_CONFIG_DEFAULT_REPORTER", env.StringifyMacro("teamcity"))],
        )
        return env

    def on_testing_line_output(self, line):
        if self.options.verbose:
            print(line, end="")

        match = TEAMCITY_RE.match(line.strip())
        if not match:
            return
        event = match.group("event")
        attrs = {k: teamcity_unescape(v) for k, v in TEAMCITY_ATTR_RE.findall(match.group("attrs"))}
        name = attrs.get("name", "")

        if event == "testFailed":
            self._failures[name] = attrs.get("message", "")
        elif event == "testIgnored":
            self._add_case(TestCase(name=name, status=TestStatus.SKIPPED))
        elif event == "testFinished":
            message = self._failures.pop(name, None)
            # the last location in the message is the failed assertion
            sources = list(SOURCE_RE.finditer(message or ""))
            source = sources[-1] if sources else None
            self._add_case(
                TestCase(
                    name=name,
                    status=TestStatus.FAILED if message is not None else TestStatus.PASSED,
                    message=message.strip() if message else None,
                    stdout=message,
                    duration=int(attrs.get("duration", 0)) / 1000,
                    source=TestCaseSource(source["file"], int(source["line"])) if source else None,
                )
            )
        elif event == "testSuiteFinished":
            self.test_suite.on_finish()

    def _add_case(self, test_case):
        self.test_suite.add_case(test_case)
        if not self.options.verbose:
            print(test_case.humanize())
