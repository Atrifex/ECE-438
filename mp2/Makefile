CXX = g++
LD = g++
CXXFLAGS = -c -g -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -MP -MMD
LDFLAGS = -std=c++1y -stdlib=libc++ -lc++abi -lpthread

all: vec_router ls_router manager_send

#vec_router: main.o monitor_neighbors.o
#$(LD) main.o monitor_neighbors.o $(LDFLAGS) -o vec_router

vec_router: main.cpp monitor_neighbors.cpp
	$(CXX) -pthread -o vec_router main.cpp monitor_neighbors.cpp

ls_router: main.cpp monitor_neighbors.cpp
	$(CXX) -pthread -o ls_router main.cpp monitor_neighbors.cpp

manager_send: manager_send.cpp
	$(CXX) -o manager_send manager_send.cpp

clean:
		rm *.o vec_router ls_router manager_send

.PHONY: all clean
