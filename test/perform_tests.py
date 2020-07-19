#!/usr/bin/env python3
from os import listdir, path
from subprocess import check_output, run

AU = u"\U0001F346"


def read_file(filename):
    with open(filename) as f:
        return f.read()


for file in sorted(listdir()):
    if file.endswith(f".{AU}"):
        print(file)
        run(f"../{AU} {file} -noe -c | clang++ -std=c++17 -o test_prog -x c++ -O3 -", shell=True)
        print('Compiled!')
        args = file.replace(AU, 'args')
        if path.exists(args):
            args = read_file(args).strip().replace('\n', ' ')
        else:
            args = ""
        inp = file.replace(AU, 'in')
        if path.exists(inp):
            inp = "< " + inp
        else:
            inp = ""
        output = check_output(f"./test_prog {args} {inp}", shell=True).decode("UTF-8")
        expected = read_file(file.replace(AU, 'out'))
        if output != expected:
            print(f"Expected output:\n{expected}\nActual output:\n{output}\n")
            raise AssertionError(f"Test {file} failed!")

check_output("rm ./test_prog", shell=True)
print("All tests passed!")
