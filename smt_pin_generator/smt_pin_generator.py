"""
Benchmark generation for SMT builder
"""

import os
import sys
import argparse
import time
import datetime
import random

default_grid_size = 50
default_min_pins_num = 3
default_max_pins_num = 30

class Cell:

    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __repr__(self):
        return "x = " + str(self.x) + " y = " + str(self.y)

    def __str__(self):
        return "x = " + str(self.x) + " y = " + str(self.y)

    def to_xml(self):
        return "\t<point x=\"" + str(self.x) + "\" y=\"" + str(self.y) +"\" layer=\"pins\" type=\"pin\" />"

def init_cell_list(grid_size):

    grid_cells = []
    for x in range(grid_size):
        for y in range(grid_size):
            cell = Cell(x, y)
            grid_cells.append(cell)

    return grid_cells

def make_output_dir(output):
    if not os.path.exists(output):
        os.makedirs(output)

def make_bench_name(grid_size, pins_num, seed):
    name = "bench_n" + str(grid_size) + "_m" + str(pins_num)

    if seed is not None:
        name += "_s" + str(seed)

    return name + ".xml"

def bench_beg(grid_size, pins_num):
    return "<net n=\"" + str(grid_size) + "\" m=\"" + str(pins_num) + "\">"

def bench_end():
    return "</net>"

def print_bench(grid_size, pins_list, seed, output):
    bench_path = output + "/" + make_bench_name(grid_size, len(pins_list), seed)

    with open(bench_path, 'w') as f:
        print(bench_beg(grid_size, len(pins_list)), file=f)
        for pin in pins_list:
            print(pin.to_xml(), file=f)
        print(bench_end(), file=f)

def check_input(grid_size, pins_num):

    is_correct = True

    if grid_size <= 0:
        print("Error: grid size <= 0")
        is_correct = False

    if pins_num > grid_size**2 or (pins_num < 0 and (grid_size**2 < default_min_pins_num or grid_size**2 < default_max_pins_num)):
        print("Error: grid is to small for pins num desired")
        is_correct = False

    if not is_correct:
        sys.exit("\nAborted\n")

def pop_random_elem(input_list):

    random_pos = random.randrange(len(input_list))

    return input_list.pop(random_pos)

def generate_pins_list(grid_cells, pins_num):
    pins_list = []

    for i in range(pins_num):
        pins_list.append(pop_random_elem(grid_cells))

    return pins_list

def generate_bench(grid_size, pins_num, seed, output):
    grid_cells = init_cell_list(grid_size)
    pins_list = generate_pins_list(grid_cells, pins_num)
    print_bench(grid_size, pins_list, seed, output)

def generate_benchmarks(grid_size, possible_pins_nums, seed, output):

    for pins_num in possible_pins_nums:
        generate_bench(grid_size, pins_num, seed, output)

if __name__ == "__main__":

    timestamp = datetime.datetime.fromtimestamp(time.time()).strftime("%Y_%m_%d__%H_%M_%S")

    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("-N", "--grid_size", type=int, default=default_grid_size,
                        help="grid size (default: " + str(default_grid_size) + ")")
    parser.add_argument("-M", "--pins_num", type=int, default=-1,
                        help="pins num (default: make all variants from " + str(default_min_pins_num) +
                             " to " + str(default_max_pins_num) + ")")
    parser.add_argument("-o", "--output", type=str, default=timestamp,
                        help="output directory (default: generated from timestamp)")
    parser.add_argument("-s", "--seed", type=int, default=None,
                        help="seed for random number generator (default: None - random)")

    args = parser.parse_args()

    check_input(args.grid_size, args.pins_num)

    random.seed(args.seed)

    make_output_dir(args.output)

    if args.pins_num < 0:
        possible_pins_nums = range(default_min_pins_num, default_max_pins_num + 1)
    else:
        possible_pins_nums = [args.pins_num]

    generate_benchmarks(args.grid_size, possible_pins_nums, args.seed, args.output)
