import time

from csp import Constraint, Variable, CSP
from constraints import *
from backtracking import bt_search
import sys
import argparse


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


def preprocess(start_board, size, varn, row_c, col_c):
    for i in range(1, size - 1):
        for j in range(1, size - 1):
            cell = varn[str(-1 - (i * size + j))]
            cell_up = varn[str(-1 - ((i - 1) * size + j))]
            cell_down = varn[str(-1 - ((i + 1) * size + j))]
            cell_left = varn[str(-1 - (i * size + j - 1))]
            cell_right = varn[str(-1 - (i * size + j + 1))]

            if start_board[i][j] == '0':
                continue

            if start_board[i][j] != '.':
                cell.setValue(1)
            else:
                cell.setValue(0)

            if start_board[i][j] == 'S':
                cell_right.setValue(0)
                cell_left.setValue(0)
                cell_up.setValue(0)
                cell_down.setValue(0)
            elif start_board[i][j] == '<':
                cell_left.setValue(0)
                cell_right.setValue(1)
                cell_up.setValue(0)
                cell_down.setValue(0)
                setdiags(varn, i, j+1)
            elif start_board[i][j] == '^':
                cell_right.setValue(0)
                cell_left.setValue(0)
                cell_up.setValue(0)
                cell_down.setValue(1)
                setdiags(varn, i+1, j)
            elif start_board[i][j] == '>':
                cell_right.setValue(0)
                cell_left.setValue(1)
                cell_up.setValue(0)
                cell_down.setValue(0)
                setdiags(varn, i, j-1)
            elif start_board[i][j] == 'v':
                cell_right.setValue(0)
                cell_left.setValue(0)
                cell_up.setValue(1)
                cell_down.setValue(0)
                setdiags(varn, i-1, j)
            elif start_board[i][j] == 'M':
                if cell_right.getValue() == cell_left.getValue() == 0:
                    cell_up.setValue(1)
                    cell_down.setValue(1)
                    setdiags(varn, i-1, j)
                    setdiags(varn, i+1, j)
                elif cell_up.getValue() == cell_down.getValue() == 0:
                    cell_left.setValue(1)
                    cell_right.setValue(1)
                    setdiags(varn, i, j-1)
                    setdiags(varn, i, j+1)

            setdiags(varn, i, j)
            check_rows_cols(size, varn, row_c, col_c)


def setdiags(varn, i, j):
    varn[str(-1 - ((i + 1) * size + j + 1))].setValue(0)
    varn[str(-1 - ((i + 1) * size + j - 1))].setValue(0)
    varn[str(-1 - ((i - 1) * size + j + 1))].setValue(0)
    varn[str(-1 - ((i - 1) * size + j - 1))].setValue(0)


def check_rows_cols(size, varn, row_c, col_c):
    for con in row_c:
        if con.satisfied():
            for var in con.unAssignedVars():
                var.setValue(0)
        elif con.cells_needed() == con.numUnassigned():
            for var in con.unAssignedVars():
                var.setValue(1)

    for con in col_c:
        if con.satisfied():
            for var in con.unAssignedVars():
                var.setValue(0)
        elif con.cells_needed() == con.numUnassigned():
            for var in con.unAssignedVars():
                var.setValue(1)

def get_ships_board(s, size):
    output_board = [[0 for col in range(size)] for row in range(size)]
    s_ = {}
    for (var, val) in s:
        s_[int(var.name())] = val
    for i in range(1, size - 1):
        for j in range(1, size - 1):
            cell = s_[-1 - (i * size + j)]
            if cell == 0:
                output_board[i][j] = '.'
                continue

            cell_up = s_[-1 - ((i - 1) * size + j)]
            cell_down = s_[-1 - ((i + 1) * size + j)]
            cell_left = s_[-1 - (i * size + j - 1)]
            cell_right = s_[-1 - (i * size + j + 1)]

            if cell_left == cell_right == cell_down == cell_up == 0:
                output_board[i][j] = 'S'
            elif cell_left == cell_right == 1 or cell_up == cell_down == 1:
                output_board[i][j] = 'M'
            elif cell_left == 1 and cell_right == 0:
                output_board[i][j] = '>'
            elif cell_left == 0 and cell_right == 1:
                output_board[i][j] = '<'
            elif cell_up == 1 and cell_down == 0:
                output_board[i][j] = 'v'
            elif cell_up == 0 and cell_down == 1:
                output_board[i][j] = '^'

    return output_board


def check_sol(s, size, ships, board, hints_board):
    correct_ships = [0] * 5
    for i in range(5):
        correct_ships[i] = int(ships[i])

    curr_ships = [0] * 5
    s_ = {}
    for (var, val) in s:
        s_[int(var.name())] = val
    for i in range(1, size - 1):
        for j in range(1, size - 1):
            cell = board[i][j]

            if cell == '.':
                continue

            if hints_board[i][j] != '0' and hints_board[i][j] != cell:
                return False

            if cell == 'S':
                curr_ships[0] += 1
            elif cell == '>' or cell == 'v':
                s = get_ship_type(board, i, j) - 1
                if s > 4:
                    return False
                curr_ships[s] += 1

    # print(correct_ships)
    # print(curr_ships)
    if curr_ships != correct_ships:
        return False

    return True


def get_ship_type(board, row, column):
    """board[row][column] """
    count = 0

    if board[row][column] == '>':
        while column != 0 and board[row][column] != '.':
            count += 1
            column -= 1
    elif board[row][column] == 'v':
        while row != 0 and board[row][column] != '.':
            count += 1
            row -= 1

    if count != 0:
        return count
    else:
        raise Exception("invalid get_ship_type input")


def print_board(s_, size):
    for i in range(1, size - 1):
        for j in range(1, size - 1):
            cell = s_[str(-1 - (i * size + j))].getValue()

            if cell is None:
                print("0", end="")
            elif cell == 0:
                print('.', end="")
            else:
                print(cell, end="")

        print('')


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

# 1/0 variables
for i in range(0, size):
    for j in range(0, size):
        v = None
        if i == 0 or i == size - 1 or j == 0 or j == size - 1:
            v = Variable(str(-1 - (i * size + j)), [0])
            v.setValue(0)
        else:
            v = Variable(str(-1 - (i * size + j)), [0, 1])
        varlist.append(v)
        varn[str(-1 - (i * size + j))] = v

# row and column constraints on 1/0 variables
row_constraint = []
row_cons = []
for i in board.split()[0]:
    row_constraint += [int(i)]

for row in range(0, size):
    con = NValuesConstraint('row', [varn[str(-1 - (row * size + col))] for col in range(0, size)],
                            [1], row_constraint[row], row_constraint[row])
    row_cons.append(con)
    conslist.append(con)

col_constraint = []
col_cons = []
for i in board.split()[1]:
    col_constraint += [int(i)]

for col in range(0, size):
    con = NValuesConstraint('col', [varn[str(-1 - (col + row * size))] for row in range(0, size)],
                            [1], col_constraint[col], col_constraint[col])
    conslist.append(con)
    col_cons.append(con)

# diagonal constraints on 1/0 variables
for i in range(1, size - 1):
    for j in range(1, size - 1):
        conslist.append(NValuesConstraint('diag', [varn[str(-1 - (i * size + j))],
                                                   varn[str(-1 - ((i - 1) * size + (j - 1)))]],
                                          [1], 0, 1))
        conslist.append(NValuesConstraint('diag', [varn[str(-1 - (i * size + j))],
                                                   varn[str(-1 - ((i - 1) * size + (j + 1)))]],
                                          [1], 0, 1))

preprocess(b3[3:], size, varn, row_cons, col_cons)
print_board(varn, size)

# find all solutions and check which one has right ship #'s
start_time = time.time()
csp = CSP('battleship', varlist, conslist)
solutions, num_nodes = bt_search('GAC', csp, 'mrv', True, False)
total_time = time.time() - start_time

print("time taken:", total_time)
print("nodes explored:", num_nodes)

sys.stdout = open(args.outputfile, 'w')
for i in range(len(solutions)):
    sol_board = get_ships_board(solutions[i], size)
    if check_sol(solutions[i], size, b2[2], sol_board, b3[3:]):
        print_solution(solutions[i], size)
