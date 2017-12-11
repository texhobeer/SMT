"""
SMT visualisation
"""

import argparse
import xml.etree.ElementTree as ET

class Color:
    red = '\033[31;41m'
    black = '\033[30;40m'
    white = '\033[37;47m'
    blue = '\033[34;44m'
    default = '\033[0m'

def colored_str(color, str):
    return color + str + Color.default

class Cell:
    def __init__(self, no_d, no_c):
        self.is_pin = False
        self.is_segment_m2 = False
        self.is_segment_m3 = False
        self.is_via = False
        self.no_duplication = no_d
        self.no_colors = no_c

    def set_segment_m2(self):
        self.is_segment_m2 = True

    def set_segment_m3(self):
        self.is_segment_m3 = True

    def set_pin(self):
        self.is_pin = True

    def set_via(self):
        self.is_via = True

    def to_colored_str(self):
        symbol = colored_str(Color.white, '0')

        if self.is_segment_m2:
            symbol = colored_str(Color.black, '2')
        if self.is_segment_m3:
            symbol = colored_str(Color.black, '3')
        if self.is_via:
            symbol = colored_str(Color.blue, 'v')
        if self.is_pin:
            symbol = colored_str(Color.red, 'p')

        if self.no_duplication:
            return symbol
        else:
            return symbol + symbol

    def to_non_colored_str(self):
        symbol = '0'

        if self.is_segment_m2:
            symbol = '2'
        if self.is_segment_m3:
            symbol = '3'
        if self.is_via:
            symbol = 'v'
        if self.is_pin:
            symbol = 'p'

        if self.no_duplication:
            return symbol
        else:
            return symbol + symbol

    def __str__(self):
        if self.no_colors:
            return self.to_non_colored_str()
        else:
            return self.to_colored_str()

    def print_descr(no_d, no_c):
        empty = Cell(no_d, no_c)
        pin = Cell(no_d, no_c)
        pin.set_pin()
        via = Cell(no_d, no_c)
        via.set_via()
        segment_m2 = Cell(no_d, no_c)
        segment_m2.set_segment_m2()
        segment_m3 = Cell(no_d, no_c)
        segment_m3.set_segment_m3()

        print()

        print(str(empty) + " - empty")

        print(str(pin) + " - pin")

        print(str(via) + " - via")

        print(str(segment_m2) + " - segment m2")

        print(str(segment_m3) + " - segment m3")

class Grid:
    def __init__(self, no_d, no_c):
        self.no_duplication = no_d
        self.no_colors = no_c

    def init_self(self, grid_size):
        self.size = grid_size
        self.cells = []

        for i in range(grid_size**2):
            self.cells.append(Cell(self.no_duplication, self.no_colors))

    def get_cell_at_pos(self, x, y):
        return self.cells[x + y * self.size]

    def parse_point(self, point):
        x = int(point.attrib['x'])
        y = int(point.attrib['y'])
        point_type = point.attrib['type']
        cell = self.get_cell_at_pos(x, y)

        if point_type == "pin":
            cell.set_pin()

        if point_type == "via":
            cell.set_via()

    def parse_segment(self, segment):
        x1 = int(segment.attrib['x1'])
        y1 = int(segment.attrib['y1'])
        x2 = int(segment.attrib['x2'])
        y2 = int(segment.attrib['y2'])
        segment_layer = segment.attrib['layer']

        if x1 < x2:
            x_min = x1
            x_max = x2
        else:
            x_min = x2
            x_max = x1

        if y1 < y2:
            y_min = y1
            y_max = y2
        else:
            y_min = y2
            y_max = y1

        for x in range(x_min, x_max + 1):
            for y in range(y_min, y_max + 1):
                cell = self.get_cell_at_pos(x, y)

                if segment_layer == "m2":
                    cell.set_segment_m2()

                if segment_layer == "m3":
                    cell.set_segment_m3()

    def parse_child(self, child):
        if child.tag == "point":
            self.parse_point(child)
        elif child.tag == "segment":
            self.parse_segment(child)

    def parse_net(self, net):
        grid_size = int(net.attrib['grid_size'])

        self.init_self(grid_size)
        for child in net:
            self.parse_child(child)

    def draw(self):
        pic = ""
        for y in range(self.size):
            for x in range(self.size):
                symbol = str(self.get_cell_at_pos(x, y))
                pic += symbol
            if y < self.size - 1:
                pic += "\n"
        print(pic)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("input", help="net xml for visualisation")
    parser.add_argument("-no_d", "--no_duplication", action="store_true",
                        help="to make picture better each symbol is printed twice,\n"
                              "this feature can be turned off by this option")
    parser.add_argument("-no_c", "--no_colors", action="store_true",
                        help="print only text output")

    args = parser.parse_args()

    net = ET.parse(args.input).getroot()

    grid = Grid(args.no_duplication, args.no_colors)

    grid.parse_net(net)

    grid.draw()

    Cell.print_descr(args.no_duplication, args.no_colors)
