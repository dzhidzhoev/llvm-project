#!/usr/bin/env python3

"""Updates FileCheck checks in GlobalISel Known Bits tests.

This script is a utility to update MIR based tests with new FileCheck
patterns for GlobalISel Known Bits.

The checks added by this script are similar to update_mir_test_checks, using
the output of KnownBits and SignBits from -passes=print<gisel-value-tracking>.
"""

from __future__ import print_function

from sys import stderr
from traceback import print_exc
import argparse
import os
import re
import sys

from UpdateTestChecks import common

VT_FUNCTION_RE = re.compile(
    r"\s*name:\s*@(?P<func>[A-Za-z0-9_-]+)"
    r"(?P<body>(\s*%[0-9a-zA-Z_]+:_\s*KnownBits:[01?]+\sSignBits:[0-9]+$)+)",
    flags=(re.X | re.M),
)


def update_test(ti: common.TestInfo):
    run_list = []
    for l in ti.run_lines:
        if "|" not in l:
            common.warn("Skipping unparsable RUN line: " + l)
            continue

        (llc_cmd, filecheck_cmd) = tuple([cmd.strip() for cmd in l.split("|", 1)])
        common.verify_filecheck_prefixes(filecheck_cmd)

        if not llc_cmd.startswith("llc "):
            common.warn("Skipping non-llc RUN line: " + l)
            continue

        if not filecheck_cmd.startswith("FileCheck "):
            common.warn("Skipping non-FileChecked RUN line: " + l)
            continue

        llc_cmd_args = llc_cmd[4:].strip()
        llc_cmd_args = llc_cmd_args.replace("< %s", "").replace("%s", "").strip()
        check_prefixes = common.get_check_prefixes(filecheck_cmd)

        run_list.append((check_prefixes, llc_cmd_args))

    ginfo = common.make_analyze_generalizer(version=1)
    builder = common.FunctionTestBuilder(
        run_list=run_list,
        flags=type(
            "",
            (object,),
            {
                "verbose": ti.args.verbose,
                "filters": ti.args.filters,
                "function_signature": False,
                "check_attributes": False,
                "replace_value_regex": [],
            },
        ),
        scrubber_args=[],
        path=ti.path,
        ginfo=ginfo,
    )

    for prefixes, llc_args in run_list:
        common.debug("Extracted llc cmd:", "llc", llc_args)
        common.debug("Extracted FileCheck prefixes:", str(prefixes))

        if ti.path.endswith(".mir"):
            llc_args += " -x mir"
        raw_tool_output = common.invoke_tool(
            ti.args.llc_binary or "llc", llc_args, ti.path, verbose=ti.args.verbose
        )

        builder.process_run_line(
            VT_FUNCTION_RE,
            common.scrub_body,
            raw_tool_output,
            prefixes,
        )

        builder.processed_prefixes(prefixes)

    func_dict = builder.finish_and_get_func_dict()
    prefix_set = set([prefix for p in run_list for prefix in p[0]])
    common.debug("Rewriting FileCheck prefixes:", str(prefix_set))
    output_lines = common.add_mir_checks(
        ti.input_lines,
        prefix_set,
        ti.test_autogenerated_note,
        ti.path,
        run_list,
        func_dict,
        print_fixed_stack=False,
        first_check_is_next=True,
        at_the_function_name=True,
    )

    common.debug("Writing %d lines to %s..." % (len(output_lines), ti.path))

    with open(ti.path, "wb") as f:
        f.writelines(["{}\n".format(l).encode("utf-8") for l in output_lines])


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        "--llc-binary",
        default=None,
        help='The "llc" binary to generate the test case with',
    )
    parser.add_argument("tests", nargs="+")
    args = common.parse_commandline_args(parser)

    script_name = os.path.basename(__file__)
    returncode = 0
    for ti in common.itertests(args.tests, parser, script_name="utils/" + script_name):
        try:
            update_test(ti)
        except Exception:
            stderr.write(f"Error: Failed to update test {ti.path}\n")
            print_exc()
            returncode = 1
    return returncode


if __name__ == "__main__":
    sys.exit(main())
