CXX = g++
LD = g++
CXXFLAGS = -c -g -std=c++11 -Wall -Wextra -pedantic -D DEBUG
LDFLAGS = -std=c++11 -pthread

all: vec_router ls_router manager_send

#vec_router: main2.o vec_router.o
#$(LD) main2.o vec_router.o $(LDFLAGS) -o vec_router

# Temporarily have vec_router = ls_router
vec_router: main.o ls_router.o router.o graph.o
	$(LD) main.o ls_router.o router.o graph.o $(LDFLAGS) -o ls_router

ls_router: main.o ls_router.o router.o graph.o
	$(LD) main.o ls_router.o router.o graph.o $(LDFLAGS) -o ls_router

manager_send: manager_send.o
	$(LD) manager_send.o $(LDFLAGS) -o manager_send

main.o: main.cpp ls_router.h router.h graph.h
	$(CXX) $(CXXFLAGS) main.cpp

manager_send.o: manager_send.cpp
	$(CXX) $(CXXFLAGS) manager_send.cpp

ls_router.o: ls_router.cpp ls_router.h router.h graph.h
	$(CXX) $(CXXFLAGS) ls_router.cpp

router.o: router.cpp router.h
	$(CXX) $(CXXFLAGS) router.cpp

graph.o: graph.cpp graph.h
	$(CXX) $(CXXFLAGS) graph.cpp

clean:
		rm -f *.o vec_router ls_router manager_send

.PHONY: all clean
