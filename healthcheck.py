#!/bin/python3

import os

program_dir = "./tests/ost/programs/"
tests_dir = "./tests/ost/tests/"

_, _, programs = next(iter(os.walk(program_dir)))
_, _, tests = next(iter(os.walk(tests_dir)))

programs.sort()
tests.sort()

for program, test in zip(programs, tests):
    exit_code = os.system(f"./build/osttest {program_dir + program} {tests_dir + test} > /dev/null")
    if exit_code == 0:
        print(f"{program}) SUCCESS")
    else:
        print(f"{program}) FAILED")

