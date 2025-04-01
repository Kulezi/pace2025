from pathlib import Path
import sys
import gurobipy as gp
from gurobipy import GRB

class Instance:
    def __init__(self, in_file):
        self.nodes = []
        self.adj = []
        self.E = 0
        with open(in_file, 'r') as f:
            for line in f:
                tokens = line.strip().split()
                if len(tokens) == 0:
                    continue

                s = tokens[0]
                if s[0] == 'c':  # comment line
                    continue
                elif s[0] == 'p':  # problem header
                    self.parse_header(tokens)
                else:  # edge line
                    a = int(s)
                    b = int(tokens[1])
                    self.E -= 1
                    self.add_edge(a, b)

        assert self.E == 0

    def parse_header(self, tokens):
        problem = tokens[1]
        n_nodes = int(tokens[2])
        self.E = int(tokens[3])

        # assert problem == "hs"
        self.adj = [[] for _ in range(n_nodes + 1)]

        for i in range(1, n_nodes + 1):
            self.nodes.append(i)

    def add_edge(self, a, b):
        self.adj[a].append(b)
        self.adj[b].append(a)


f = open("results.csv", "w")

def solve_milp(instance_file_path):
    g = Instance(instance_file_path)
    print(g)

    m = gp.Model(name=instance_file_path.name)

    is_selected = m.addVars(g.nodes, vtype=GRB.BINARY, name='is_selected')

    for v in g.nodes:
        m.addConstr(sum(is_selected[u] for u in g.adj[v]) + is_selected[v] >= 1)

    m.setObjective(sum(is_selected[v] for v in g.nodes), GRB.MINIMIZE)

    m.setParam('LogToConsole', 0)
    m.setParam('TimeLimit', 60)
    m.setParam('LogFile', "gurobi2.log")
    m.optimize()

    print(instance_file_path.name, int(sum(is_selected[v] for v in g.nodes).getValue()), m.Runtime, m.Status, sep=",", file=f, flush=True)
    m.write(instance_file_path.name + ".mps")

directory_path = Path("../in/PACE2025-instances/ds/exact")

# Iterating over files in a directory
for file in directory_path.iterdir():
    if file.name.endswith(".lp"):
        continue
    if file.is_file():  # Check if it's a file (not a directory)
        solve_milp(file)
