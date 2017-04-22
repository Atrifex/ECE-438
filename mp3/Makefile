CXX = g++
LD = g++
CXXFLAGS = -c -g -std=c++11 -Wall -Wextra -pedantic
LDFLAGS = -std=c++11 -pthreads

all: reliable_sender reliable_receiver

reliable_sender: sender_main.o
	$(LD) sender_main.o $(LDFLAGS) -o reliable_sender

reliable_receiver: receiver_main.o
	$(LD) receiver_main.o $(LDFLAGS) -o reliable_receiver

sender_main.o: sender_main.cpp
	$(CXX) $(CXXFLAGS) sender_main.cpp

receiver_main.o: receiver_main.cpp
	$(CXX) $(CXXFLAGS) receiver_main.cpp

clean:
	rm -f reliable_sender reliable_receiver
