#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

void listenForNeighbors();
void* announceToNeighbors(void* unusedParam);

int globalMyID = 0;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
struct sockaddr_in globalNodeAddrs[256];

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
		exit(1);
	}

	//initialization: get this process's node ID, record what time it is,
	//and set up our sockaddr_in's for sending to the other nodes.
	globalMyID = atoi(argv[1]);
	int i, j;
	for(i=0;i<256;i++)
	{
		gettimeofday(&globalLastHeartbeat[i], 0);

		char tempaddr[100];
		sprintf(tempaddr, "10.1.1.%d", i);
		memset(&globalNodeAddrs[i], 0, sizeof(globalNodeAddrs[i]));
		globalNodeAddrs[i].sin_family = AF_INET;
		globalNodeAddrs[i].sin_port = htons(7777);
		inet_pton(AF_INET, tempaddr, &globalNodeAddrs[i].sin_addr);
	}

	// TODO: Create Graph object with all invalid entries with cost 1

	//TODO: read and parse initial costs file. default to cost 1 if no entry for a node. file may be empty.
	// Note: below code will end up going in the graph constructor
	int j, k;
	int num_bytes;
	int bytes_read;
	char * initialcostsfile = (char*)argv[2];
	FILE * fp = fopen(initialcostsfile, "r");

	if(fp == NULL)
	{
		fprintf("Erroneous initial costs file provided\n");
		exit(1);
	}

	// Check for an empty file
	int filesize = fseek(fp, 0, SEEK_END); // size is 0 if empty
  rewind(fp);

	if(filesize != 0)
	{
		char cur_node[4];
		char cur_cost[8]; // Cost must be less than 2^23
		char * cur_link = (char*) malloc((num_bytes + 1)*sizeof(char));
		while((bytes_read = getline(&cur_link, &num_bytes, fp)) != -1) // Read a line from the initial costs file
		{
				// Parse each line individually, filling out the cur_node and cur_cost strings
				j = 0;
				while(cur_link[j] != ' ')
				{
					cur_node[j] = cur_link[j];
					j++;
				}
				cur_node[j] = '\0'; // Assume they aren't trying to break stuff

				j++;
				k = 0;
				while(cur_link[j] != '\n')
				{
					cur_cost[k] = cur_link[j];
					k++; j++;
				}
				cur_cost[k] = '\0'; // Again, assume they aren't trying to break things

				int node_int = atoi(cur_node);
				int cost_int = atoi(cur_cost);

				// Update link cost in the graph accordingly
		}

		free(cur_link);
	}

	//socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
	if((globalSocketUDP=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	char myAddr[100];
	struct sockaddr_in bindAddr;
	sprintf(myAddr, "10.1.1.%d", globalMyID);
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(7777);
	inet_pton(AF_INET, myAddr, &bindAddr.sin_addr);
	if(bind(globalSocketUDP, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind");
		close(globalSocketUDP);
		exit(1);
	}

	//start threads... feel free to add your own, and to remove the provided ones.
	pthread_t announcerThread;
	pthread_create(&announcerThread, 0, announceToNeighbors, (void*)0);

	//good luck, have fun!
	listenForNeighbors();
}
