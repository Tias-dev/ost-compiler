#!/bin/python3

import os
import re

program_dir = "./tests/ost/programs/"
tests_dir = "./tests/ost/tests/"

_, _, programs = next(iter(os.walk(program_dir)))
_, _, tests = next(iter(os.walk(tests_dir)))


def getMTName(file):
	with open(file, "r") as f:
		text = f.read();
		match = re.findall(r"MT (\w+);", text)
		if len(match) == 0:
			return None
		return match[0]


programs.sort()
tests.sort()

for program, test in zip(programs, tests):
	mtName = getMTName(program_dir + program)
	exit_code = os.system(f"./build/ost {program_dir + program} -o /tmp > /dev/null")
	exit_code = os.system(f"./build/osttest /tmp/{mtName}.tu4 {tests_dir + test} > /dev/null")
	if exit_code == 0:
			print(f"{program}) SUCCESS")
	else:
			print(f"{program}) FAILED")
