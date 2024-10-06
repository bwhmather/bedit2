"""
Checks that all source and header files directly import the headers that they
need.
"""
import difflib
import itertools
import re
import sys

from bedit_lint import INCLUDE_ROOT, PROJECT_ROOT, enumerate_header_paths


def test():
    passed = True

    for header_path in itertools.chain(enumerate_header_paths()):
        with header_path.open() as header:
            actual_guard = [
                header.readline(),
                header.readline(),
            ]

            actual_end = []
            for line in header.readlines():

                actual_end = [*actual_end[-1:],line]

        guard_name = re.sub("[-/.]", "_", str(header_path.relative_to(INCLUDE_ROOT)).upper())

        expected_guard = [
            f"#pragma once\n",
            "\n",
        ]

        expected_end = [
            "\n",
            "G_END_DECLS\n",
        ]

        if actual_guard != expected_guard or actual_end != expected_end:
            msg = "======================================================================\n"
            msg += f"FAIL: test_include_guards: {header_path.relative_to(PROJECT_ROOT)}\n"
            msg += "----------------------------------------------------------------------\n"

            if actual_guard != expected_guard:
                msg += "Header include guards do not match expected:\n"
                for diff_line in difflib.ndiff(expected_guard, actual_guard):
                    if diff_line.startswith("?"):
                        continue
                    msg += f"  {diff_line}"
                msg += "\n"

            if actual_end != expected_end:
                msg += "Header end do not match expected:\n"
                for diff_line in difflib.ndiff(expected_end, actual_end):
                    if diff_line.startswith("?"):
                        continue
                    msg += f"  {diff_line}"
                msg += "\n"

            print(msg, file=sys.stderr)
            passed = False

    return passed


if __name__ == "__main__":
    sys.exit(0 if test() else 1)
