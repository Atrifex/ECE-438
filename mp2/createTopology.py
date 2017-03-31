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

##### Generate Graph #####
G=nx.fast_gnp_random_graph(20, .1)

# assigns random weights to all of the edges
for (u, v) in G.edges():
    G.edge[u][v]['weight'] = random.randint(0,100)

##### Setup Enviornment ####
if os.path.isdir("./topology"):
    shutil.rmtree("./topology")
os.mkdir("./topology")

##### Output Files #####
# k controls the distance between the nodes and varies between 0 and 1
# iterations is the number of times simulated annealing is run
pos=nx.spring_layout(G,k=1,iterations=50)
nx.draw(G,pos,node_color='#A0CBE2',width=.5,with_labels=True)
#nx.draw_networkx_edge_labels(G,pos=pos)
plt.savefig("./topology/topology.png")  # save as png

# Topology for IP tables
nx.write_edgelist(G,"./topology/networkTopology",data=False)

# Write initial costs to file
for v in G:
    initCostFile = open("./topology/node" + str(v) + "costs", 'w')
    for n in G.neighbors(v):
        initCostFile.write(str(n) + " " + str(G[v][n]['weight']) + "\n")
