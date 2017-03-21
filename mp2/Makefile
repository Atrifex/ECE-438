CXX = g++
LD = g++
CXXFLAGS = -g -std=c++1y -Wall -Wextra -pedantic -MP -MMD
LDFLAGS = -std=c++1y -lc++abi -lpthread

all: vec_router ls_router manager_send

#vec_router: main.o monitor_neighbors.o
#$(LD) main.o monitor_neighbors.o $(LDFLAGS) -o vec_router

vec_router: main.cpp monitor_neighbors.cpp graph.cpp
	$(CXX) -pthread $(CXXFLAGS) -o vec_router main.cpp monitor_neighbors.cpp graph.cpp

ls_router: main.cpp monitor_neighbors.cpp graph.cpp
	$(CXX) -pthread $(CXXFLAGS) -o ls_router main.cpp monitor_neighbors.cpp graph.cpp

manager_send: manager_send.cpp
	$(CXX) -o manager_send manager_send.cpp

clean:
		rm -f *.o vec_router ls_router manager_send

.PHONY: all clean
