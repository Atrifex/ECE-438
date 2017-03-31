#!/usr/bin/env python
"""
You must have networkx, matplotlib>=87.7 for this program to work.
"""
# Author: Rishi Thakkar (rishirt.us@gmail.com)
try:
    import matplotlib.pyplot as plt
except:
    raise

import networkx as nx
import random
import sys
import os
import shutil

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

##### Get user inputs #####
print("Welcome to the Graph Generator!\n")

print(bcolors.HEADER + bcolors.BOLD + bcolors.UNDERLINE + "Graph Type to Use:" + bcolors.ENDC)

modeDescription = bcolors.WARNING + "Mode 0 - " + bcolors.ENDC + bcolors.OKGREEN + "Random Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 1 - " + bcolors.ENDC + bcolors.OKGREEN + "Complete Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 2 - " + bcolors.ENDC + bcolors.OKGREEN + "Barbell Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 3 - " + bcolors.ENDC + bcolors.OKGREEN + "2D Grid Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 4 - " + bcolors.ENDC + bcolors.OKGREEN + "Dorogovtsev Goltsev Mmendes Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 5 - " + bcolors.ENDC + bcolors.OKGREEN + "Cycle Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 6 - " + bcolors.ENDC + bcolors.OKGREEN + "Circular Ladder Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 7 - " + bcolors.ENDC + bcolors.OKGREEN + "Lollipop Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 8 - " + bcolors.ENDC + bcolors.OKGREEN + "Wheel Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 9 - " + bcolors.ENDC + bcolors.OKGREEN + "Star Graph" + bcolors.ENDC + "\n"
modeDescription += bcolors.WARNING + "Mode 10 - " + bcolors.ENDC + bcolors.OKGREEN + "Path Graph" + bcolors.ENDC + "\n"
print(modeDescription)

##### Generate Graph #####
while(1):
    mode = int(input("Please enter mode of graph type for generation: "))
    if mode == 0:
        nodes = int(input("Number of nodes: "))
        edgeP = float(input("Probability of edge formation: "))
        G=nx.fast_gnp_random_graph(nodes, edgeP)
        pos=nx.spring_layout(G,k=1,iterations=100)
        break
    elif mode == 1:
        nodes = int(input("Number of nodes: "))
        G=nx.complete_graph(nodes)
        pos=nx.spring_layout(G,k=1,iterations=100)
        break
    elif mode == 2:
        nodesL = int(input("Number of outer nodes (>= 1): "))
        nodesR = int(input("Number of nodes for connections: "))
        G=nx.barbell_graph(nodesL, nodesR)
        pos=nx.spring_layout(G,k=1,iterations=100)
        break
    elif mode == 3:
        rows = int(input("Number of rows: "))
        cols = int(input("Number of cols: "))
        G=nx.grid_2d_graph(rows, cols)
        pos=nx.spectral_layout(G)
        break
    elif mode == 4:
        nodes = int(input("Number of generations (<= 5): "))
        if nodes > 5:
            print("Invalid input! Please execute script again.")
            sys.exit();
        G=nx.dorogovtsev_goltsev_mendes_graph(nodes)
        pos=nx.spring_layout(G,k=1,iterations=100)
        break
    elif mode == 5:
        nodes = int(input("Number of nodes: "))
        G=nx.cycle_graph(nodes)
        pos=nx.circular_layout(G)
        break
    elif mode == 6:
        nodes = int(input("Number of nodes: "))
        G=nx.circular_ladder_graph(nodes)
        pos=nx.spring_layout(G,k=1,iterations=100)
        break
    elif mode == 7:
        nodesK = int(input("Number of nodes in candy: "))
        nodesP = int(input("Number of nodes in stick: "))
        G=nx.lollipop_graph(nodesK, nodesP)
        pos=nx.spring_layout(G,k=1,iterations=100)
        break
    elif mode == 8:
        nodes = int(input("Number of nodes: "))
        G=nx.wheel_graph(nodes)
        pos=nx.spectral_layout(G)
        break
    elif mode == 9:
        nodes = int(input("Number of nodes: "))
        G=nx.star_graph(nodes)
        pos=nx.spring_layout(G,k=1,iterations=100)
        break
    elif mode == 10:
        nodes = int(input("Number of nodes: "))
        G=nx.path_graph(nodes)
        pos=nx.circular_layout(G)
        break
    else:
        print("Please enter a valid number.")

# assigns random weights to all of the edges
for (u, v) in G.edges():
    G.edge[u][v]['weight'] = random.randint(0,500)

##### Setup Enviornment ####
if os.path.isdir("./topology"):
    shutil.rmtree("./topology")
os.mkdir("./topology")

##### Output Files #####
plt.figure(1,figsize=(10,10))
nx.draw(G,pos,node_color='#A0CBE2',width=.5,with_labels=True)
plt.savefig("./topology/networkTopology.png")  # save as png

# Topology for IP tables, grid graph is special case
if mode != 3:
    nx.write_edgelist(G,"./topology/networkTopology.txt",data=False)
else:
    edgeList =  open("./topology/networkTopology.txt", 'w')
    for v in G:
        for n in G.neighbors(v):
            edgeList.write(str(v[0]*cols + v[1]) + " " + str(n[0]*cols + n[1]) + "\n")

# Write initial costs to file, grid graph is special case
if mode != 3:
    for v in G:
        initCostFile = open("./topology/nodecosts" + str(v), 'w')
        for n in G.neighbors(v):
            initCostFile.write(str(n) + " " + str(G[v][n]['weight']) + "\n")
else:
    for v in G:
        initCostFile = open("./topology/nodecosts" + str(v[0]*cols + v[1]), 'w')
        for n in G.neighbors(v):
            initCostFile.write(str(n[0]*cols + n[1]) + " " + str(G[v][n]['weight']) + "\n")
