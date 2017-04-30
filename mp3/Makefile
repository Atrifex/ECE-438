CXX = g++
LD = g++
CXXFLAGS = -c -g -std=c++11 -Wall -Wextra -pedantic
LDFLAGS = -std=c++11 -pthread

LIBFILES = types.h parameters.h
SENDER_OBJFILES = sender_main.o tcp.o circular_buffer.o
RECEIVER_OBJFILES = receiver_main.o tcp.o circular_buffer.o

all: reliable_sender reliable_receiver

reliable_sender: $(SENDER_OBJFILES) $(LIBFILES)
	$(LD) $(SENDER_OBJFILES) $(LDFLAGS) -o reliable_sender

reliable_receiver: $(RECEIVER_OBJFILES) $(LIBFILES)
	$(LD) $(RECEIVER_OBJFILES) $(LDFLAGS) -o reliable_receiver

sender_main.o: sender_main.cpp $(LIBFILES)
	$(CXX) $(CXXFLAGS) sender_main.cpp

receiver_main.o: receiver_main.cpp $(LIBFILES)
	$(CXX) $(CXXFLAGS) receiver_main.cpp

tcp.o: tcp.cpp tcp.h $(LIBFILES)
	$(CXX) $(CXXFLAGS) tcp.cpp

circular_buffer.o: circular_buffer.cpp circular_buffer.h $(LIBFILES)
	$(CXX) $(CXXFLAGS) circular_buffer.cpp

clean:
	rm -f reliable_sender reliable_receiver *.o
