#!/usr/bin/python

#to install networkx, use following commands:
#	sudo apt-get install python-pip
#	sudo pip install -U pip setuptools
#	sudo pip install networkx

import sys
import random
import itertools
import time
import os
import fcntl
import networkx as nx
from subprocess import call, check_output, CalledProcessError, Popen, PIPE
from asyncproc import Process


executable = 'ls_router'

def findPID(nodeid, executable):
	command = "ps aux | grep \"{}\" | grep \" {} \" | grep -v grep".format(executable, nodeid)
	try:
		output = check_output(command, shell=True)
		return output.split()[1]
	except CalledProcessError:
		print "cannot find {} process with id {}".format(executable, nodeid)
		return -1

def compile():
	call("make clean && make", shell=True)

def setup_network(folder_name, topofilename):
	call("perl make_topology.pl {}/{}".format(folder_name, topofilename), shell=True)

def addlink(node1, node2):
	command1 = "sudo iptables -v -I OUTPUT 1 -s 10.1.1.{} -d 10.1.1.{} -j ACCEPT".format(node1, node2)
	call(command1, shell=True)
	
def removelink(node1, node2):
	command1 = "sudo iptables -v -D OUTPUT -s 10.1.1.{} -d 10.1.1.{} -j ACCEPT".format(node1, node2)
	call(command1, shell=True)

def run_routers(nodes, folder_name, initcost_prefix):
	channels = {}
	call("pkill {}".format(executable), shell=True)
	call("rm -rf ./log", shell=True)
	call("mkdir log", shell=True)
	for node in nodes:
		call("./{binary} {id} {folder}/{prefix}{id} ./log/log{id} &".format(
			binary=executable, id=node, folder=folder_name, prefix=initcost_prefix), 
			shell=True)
	time.sleep(0.3)
	for node in nodes:
		sp = Process("tail -f ./log/log{}".format(node), shell=True)
		channels[node] = sp
	return channels

def parse_topo(folder_name, initcost_prefix, topofilename):
	topo = nx.DiGraph()
	linkcosts = {}
	edges = []
	files = os.listdir(folder_name)
	for filename in files:
		if filename.find(initcost_prefix) is 0:
			nodeid = int(filename[len(initcost_prefix):])
			linkcosts[nodeid] = {}
			with open(folder_name+'/'+filename, 'r') as costfile:
				for line in costfile:
					[destid, cost] = [int(x) for x in line.split(' ')]
					linkcosts[nodeid][destid] = cost
	print linkcosts

	for node in linkcosts.keys():
		topo.add_node(node)

	with open(folder_name+'/'+topofilename, 'r') as topofile:
		for line in topofile:
			[id1, id2] = [int(x) for x in line.split(' ')]
			length12 = 1 if (linkcosts[id1].get(id2) is None) else linkcosts[id1][id2]
			length21 = 1 if (linkcosts[id2].get(id1) is None) else linkcosts[id2][id1]
			topo.add_edge(id1, id2, weight=length12)
			topo.add_edge(id2, id1, weight=length21)
			edges.append([id1, id2])
	return topo, linkcosts, edges

def find_path_local(topo, source):
	inf = sys.maxint/3
	tentative = {}
	confirmed = {}
	for node in topo.nodes():
		tentative[node] = [inf, -1, -1]
	tentative[source] = [0, source, source]

	confirmed[source] = tentative.pop(source)
	last = source
	while(len(tentative) != 0):
		curr = -1
		tents = tentative.keys()
		tents.sort()
		for node in tents:
			#if there is edge, maybe update path
			if topo.edge[last].get(node) is not None:
				newdist = confirmed[last][0] + topo.edge[last][node]['weight']
				newdirecthop = confirmed[last][2]

				if newdirecthop == source:
					newdirecthop = node

				if (newdist == tentative[node][0]) and last < tentative[node][1]:
					tentative[node][0] = newdist
					tentative[node][1] = last
					tentative[node][2] = newdirecthop
				elif newdist < tentative[node][0]:
					tentative[node][0] = newdist
					tentative[node][1] = last
					tentative[node][2] = newdirecthop


			#find node to put in confirmed
			if curr==-1:
				if tentative[node][0] < inf:
					curr = node
			else:
				if tentative[node][0] < tentative[curr][0]:
					curr = node
		if curr == -1:
			break
		else:
			confirmed[curr] = tentative.pop(curr)
			last = curr
	nexthops = {}
	for dest in confirmed.keys():
		nexthops[dest] = confirmed[dest][2]
	return nexthops

def find_all_paths(topo):
	all_nexthops = {}
	for node in topo.nodes():
		all_nexthops[node] = find_path_local(topo, node)
	return all_nexthops

def find_path(all_nexthops, source, target):
	path = [source]
	last_visited = source
	while((last_visited != None) and (last_visited != target)):
		last_visited = all_nexthops[last_visited].get(target)
		if last_visited != None:
			path.append(last_visited)
	return path

def send_msg_all(channels, fromid):
	hops = {}
	for dest in channels.keys():
		if dest == fromid:
			continue
		print "manager sending message from {} to {}".format(fromid, dest)
		call("./manager_send {} send {} {}".format(fromid, dest, dest), shell=True)
		hops[dest] = {}
	time.sleep(2)

	for node in channels.keys():
		lines = channels[node].read().split('\n')
		for line in lines:
			log = line.split()
			if log == []:
				continue
			dest = int(log[-1])
			if len(log) == 0:
				continue
			elif log[0] == 'unreachable':
				hops[dest][node] = -1
			elif log[0] == 'receive':
				hops[dest][node] = node
			elif log[4] == 'nexthop':
				hops[dest][node] = int(log[5])
			else:
				print "illegal log:"
				print log
				print "lines:"
				print lines
	return hops

def all_to_all_msg(topo, channels):
	all_nexthops = find_all_paths(topo)

	nodes = channels.keys()
	for node1 in nodes:
		all_hops = send_msg_all(channels, node1)
		for node2 in nodes:
			if node1 == node2:
				continue
			hops = all_hops[node2]
			path = find_path(all_nexthops, source=node1, target=node2)

			wrong = False
			for i in range(len(path)-1):
				if (hops.get(path[i]) is None) or (hops[path[i]] != path[i+1]):
					wrong = True
			if wrong:
				print "found a wrong case: "
				print topo.edges(data=True)
				print "sending from {} to {}".format(node1, node2)
				print "should have: "
				print path
				print "but have: "
				print hops
				call("pkill ls_router", shell=True)
				exit(1)


def del_test_add(topo, channels, edges):
	print "----------------------------------------------------------"
	print "test case for removing: "
	print edges

	print "setting up environment: "
	weights = []
	for e in edges:
		removelink(e[0], e[1])
		weights.append(topo.edge[e[0]][e[1]]['weight'])
		topo.remove_edge(e[0], e[1])
		removelink(e[1], e[0])
		weights.append(topo.edge[e[1]][e[0]]['weight'])
		topo.remove_edge(e[1], e[0])

	time.sleep(5)
	
	print "testing: "
	all_to_all_msg(topo, channels)
	print "testing succeed"

	print "restoring environment: "
	i=0
	for e in edges:
		addlink(e[0], e[1])
		topo.add_edge(e[0], e[1], weight=weights[i])
		
		addlink(e[1], e[0])
		topo.add_edge(e[1], e[0], weight=weights[i+1])
		i+=2
	print "test case finished"

def main():
	print "the script works with only undirectional topology and link cost for now"
	if len(sys.argv) != 4:
		print "./topotest.py folder initcost_prefix topofilename"
		print "example: ./topotest.py example_topology test2initcosts topoexample.txtju"
		exit(1)

	folder_name = sys.argv[1]
	initcost_prefix = sys.argv[2]
	topofilename = sys.argv[3]

	#folder_name = 'example_topology'
	#initcost_prefix = 'test2initcosts'
	#topofilename = 'topoexample.txt'

	topo, initcosts, all_edges = parse_topo(folder_name, initcost_prefix, topofilename)
	compile()
	setup_network(folder_name, topofilename)
	channels = run_routers(initcosts.keys(), folder_name, initcost_prefix)

	#test one special cast
	#special_edges = [[1, 2], [2, 3], [3, 4], [5, 1], [6, 5]]
	#special_edges = []
	#del_test_add(topo, channels, special_edges)

	#no failure test
	#time.sleep(5)
	#all_to_all_msg(topo, channels)

	#undirected edges
	pair_edges = len(all_edges)
	all_combs = []
	for n in range(pair_edges+1):
		comb_edges_n = itertools.combinations(all_edges, n)
		for n_edges in comb_edges_n:
			all_combs.append(n_edges)

	random.shuffle(all_combs)
	for comb in all_combs:
		del_test_add(topo, channels, comb)


if __name__ == '__main__':
	main()
