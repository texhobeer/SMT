"""
Collect statistics for SMT
"""

import os
import argparse
import re
import sys
import glob
from subprocess import Popen, PIPE

def make_output_dir(output):
    if not os.path.exists(output):
        os.makedirs(output)

def check_input(input):
    is_correct = True

    if not os.path.exists(input):
        print("Error: couldn't found input directory")
        is_correct = False

    if is_correct and not os.path.isdir(input):
        print("Error: input is not a directory")
        is_correct = False

    if not is_correct:
        sys.exit("Aborted")

def get_bench_names(input):
    paths = glob.glob(input + "/*.xml")
    names = []
    for path in paths:
        m = re.search(".*/(.*)\.xml$", path)
        names.append(m.group(1))
    return names

def run_bench(smt_builder, input, bench_name, output):
    command = smt_builder + " " + input + "/" + bench_name + ".xml " + output + "/" + bench_name + "_sol.xml"
    out, err = Popen( command, shell=True, stdout=PIPE).communicate()
    print(out.decode('UTF-8'))

def run_benchmarks(smt_builder, input, output):
    names = get_bench_names(input)

    for name in names:
        run_bench( smt_builder, input, name, output)

if __name__ == "__main__":

    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("input", help="directory with benchmarks")
    parser.add_argument("output", help="directory for solutions")
    parser.add_argument("-s", "--summary_file", type=str, default="summary",
                        help="summary_file")

    args = parser.parse_args()

    check_input(args.input)
    make_output_dir(args.output)

    run_benchmarks("./main.out", args.input, args.output)
