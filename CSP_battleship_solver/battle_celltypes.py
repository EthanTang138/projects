from csp import Constraint, Variable, CSP
from constraints import *
from backtracking import bt_search
import sys
import argparse


def print_solution_big_domain(s, size):
    s_ = {}
    for (var, val) in s:
        s_[int(var.name())] = val
    for i in range(1, size - 1):
        for j in range(1, size - 1):
            print(s_[1 + (i * size + j)], end="")
        print('')


def print_solution(s, size):
    s_ = {}
    for (var, val) in s:
        s_[int(var.name())] = val
    for i in range(1, size - 1):
        for j in range(1, size - 1):
            cell = s_[-1 - (i * size + j)]
            if cell == 0:
                print(".", end="")
                continue

            cell_up = s_[-1 - ((i - 1) * size + j)]
            cell_down = s_[-1 - ((i + 1) * size + j)]
            cell_left = s_[-1 - (i * size + j - 1)]
            cell_right = s_[-1 - (i * size + j + 1)]

            if cell_left == cell_right == cell_down == cell_up == 0:
                print("S", end="")
            elif cell_left == cell_right == 1 or cell_up == cell_down == 1:
                print("M", end="")
            elif cell_left == 1 and cell_right == 0:
                print(">", end="")
            elif cell_left == 0 and cell_right == 1:
                print("<", end="")
            elif cell_up == 1 and cell_down == 0:
                print("v", end="")
            elif cell_up == 0 and cell_down == 1:
                print("^", end="")

        print('')


def preprocess(s, size):
    s_ = {}
    for (var, val) in s:
        s_[int(var.name())] = val
    for i in range(1, size - 1):
        for j in range(1, size - 1):
            cell = s_[-1 - (i * size + j)]

            cell_up = s_[-1 - ((i - 1) * size + j)]
            cell_down = s_[-1 - ((i + 1) * size + j)]
            cell_left = s_[-1 - (i * size + j - 1)]
            cell_right = s_[-1 - (i * size + j + 1)]


# parse board and ships info
# file = open(sys.argv[1], 'r')
# b = file.read()
parser = argparse.ArgumentParser()
parser.add_argument(
    "--inputfile",
    type=str,
    required=True,
    help="The input file that contains the puzzles."
)
parser.add_argument(
    "--outputfile",
    type=str,
    required=True,
    help="The output file that contains the solution."
)
args = parser.parse_args()
file = open(args.inputfile, 'r')
b = file.read()
b2 = b.split()
size = len(b2[0])
size = size + 2
b3 = []
b3 += ['0' + b2[0] + '0']
b3 += ['0' + b2[1] + '0']
b3 += [b2[2] + ('0' if len(b2[2]) == 3 else '')]
b3 += ['0' * size]
for i in range(3, len(b2)):
    b3 += ['0' + b2[i] + '0']
b3 += ['0' * size]
board = "\n".join(b3)

varlist = []
varn = {}
conslist = []

# ./S/</>/v/^/M variables
# these would be added to the csp as well, before searching,
# along with other constraints
for i in range(0, size):
    for j in range(0, size):
        if i == 0 or i == size - 1 or j == 0 or j == size - 1:
            v = Variable(str(1 + (i * size + j)), ['.'])
        else:
            v = Variable(str(i * size + j + 1), ['.', 'S', '<', '^', 'v', 'M', '>'])
        varlist.append(v)
        varn[str(str(i * size + j + 1))] = v

# make W/S/L/R/B/T/M variables match board info
ii = 0
for row in board.split()[3:]:
    jj = 0
    for cell in row:
        if cell != '0':
            conslist.append(
                TableConstraint('boolean_match', [varn[str(1 + (ii * size + jj))]], [[cell]]))
        jj += 1
    ii += 1

# row and column constraints on W/S/L/R/B/T/M variables
row_constraint = []
for i in board.split()[0]:
    row_constraint += [int(i)]

for row in range(0, size):
    conslist.append(
        NValuesConstraint('row', [varn[str(1 + (row * size + col))] for col in range(0, size)],
                          ['S', '<', '^', 'v', 'M', '>'], row_constraint[row], row_constraint[row]))

col_constraint = []
for i in board.split()[1]:
    col_constraint += [int(i)]

for col in range(0, size):
    conslist.append(
        NValuesConstraint('col', [varn[str(1 + (col + row * size))] for row in range(0, size)],
                          ['S', '<', '^', 'v', 'M', '>'], col_constraint[col], col_constraint[col]))

# diagonal constraints on W/S/L/R/B/T/M variables
for i in range(1, size - 1):
    for j in range(1, size - 1):
        conslist.append(NValuesConstraint('diag', [varn[str(1 + (i * size + j))],
                                                   varn[str(1 + ((i - 1) * size + (j - 1)))]],
                                          ['S', '<', '^', 'v', 'M', '>'], 0, 1))
        conslist.append(NValuesConstraint('diag', [varn[str(1 + (i * size + j))],
                                                   varn[str(1 + ((i - 1) * size + (j + 1)))]],
                                          ['S', '<', '^', 'v', 'M', '>'], 0, 1))

valid_pairs_h = [['.', '.'], ['.', '<'], ['.', '^'], ['.', 'v'], ['.', 'S'],
                 ['S', '.'], ['<', 'M'], ['<', '>'], ['>', '.'],
                 ['^', '.'], ['v', '.'],
                 ['M', '>'], ['.', 'M'], ['M', '.']
                 ]

valid_pairs_v = [['.', '.'], ['.', '<'], ['.', '>'], ['.', 'v'], ['.', 'S'],
                 ['.', 'M'], ['M', '.'],
                 ['S', '.'], ['v', 'M'], ['M', '^'], ['^', '.'], ['v', '^'],
                 ['>', '.'], ['<', '.'],

                 ]
# ship constraints on horizontal cells
for i in range(1, size - 1):
    for j in range(0, size - 1):
        # check left cell is water
        conslist.append(TableConstraint('hships', [varn[str(1 + (i * size + j))],
                                                   varn[str(1 + (i * size + j + 1))]],
                                        valid_pairs_h))
# ship constraints on vertical cells
for i in range(1, size):
    for j in range(1, size - 1):
        # check left cell is water
        conslist.append(TableConstraint('vships', [varn[str(1 + (i * size + j))],
                                                   varn[str(1 + ((i - 1) * size + j))]],
                                        valid_pairs_v))

# find all solutions and check which one has right ship #'s
csp = CSP('battleship', varlist, conslist)
solutions, num_nodes = bt_search('FC', csp, 'mrv', True, False)
print("nodes explored:", num_nodes)
print(solutions)
sys.stdout = open(args.outputfile, 'w')
for i in range(len(solutions)):
    print_solution_big_domain(solutions[i], size)
    print("--------------")
